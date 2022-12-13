#include "member_api_handler.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <iterator>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../client_request_sender.h"
#include "../filesystem/filesystem_utils.h"
#include "../ml_cluster/ml_utils.h"
#include "../socket_message_utils.h"
#include "../utils.h"
#include "member_const.h"
#include "member_get_id_request.h"
#include "member_get_id_response.h"
#include "member_ping_request.h"
#include "member_request.h"
#include "member_server_context.h"
#include "member_set_ringmap_request.h"
#include "member_traffic_monitor.h"
#include "member_utils.h"

using namespace std;

#define PORT 8088

// Hostname to IP address, cited:
// https://stackoverflow.com/questions/9400756/ip-address-from-host-name-in-windows-socket-programming
string HostToIp(const std::string& host) {
  hostent* hostname = gethostbyname(host.c_str());
  if (hostname) return string(inet_ntoa(**(in_addr**)hostname->h_addr_list));
  return {};
}

uint64_t getNow() {
  return (chrono::system_clock::now().time_since_epoch()).count();
}

vector<string> get_k_successor_member(string member, int k) {
  vector<string> ping_members;

  auto iter = MemberServerContext::RingMap.find(member);
  if (iter != MemberServerContext::RingMap.end()) {
    iter++;
    while (ping_members.size() < k) {
      if (iter == MemberServerContext::RingMap.end()) {
        iter = MemberServerContext::RingMap.begin();
      }

      if (*iter == member) break;

      ping_members.push_back(*iter);

      iter++;
    }
  }
  return ping_members;
}

Member parse_member_id(string member_id) {
  Member member;
  vector<string> tokens;
  size_t pos = 0;
  while ((pos = member_id.find(MEMBER_ID_DELIMITER)) != std::string::npos) {
    string token(member_id.substr(0, pos));
    tokens.push_back(token);
    member_id.erase(0, pos + MEMBER_ID_DELIMITER.length());
  }
  if (tokens.size() == 2) {
    member.port = member_id;  // input string remains port
    member.hostname = tokens.back();
  } else {
    throw runtime_error("Error occurs when parsing member ID");
  }
  return member;
}

string find_cycle_entry(unordered_map<string, string>& topology, string curr) {
  unordered_set<string> visited;

  for (int step = 0; step < topology.size(); step += 1) {
    if (visited.count(curr) > 0) return curr;
    visited.insert(curr);

    if (topology.count(curr) == 0) break;

    curr = topology[curr];
  }

  return "";
}

void fix_topology(unordered_map<string, string>& topology) {
  unordered_map<string, int> indegrees;

  // count indegrees
  for (auto const& [src, dst] : topology) {
    if (indegrees.count(dst) == 0) {
      indegrees[dst] = 0;
    }
    if (indegrees.count(src) == 0) {
      indegrees[src] = 0;
    }

    indegrees[dst] += 1;
  }

  // fix topology
  for (auto const& [member, indegree] : indegrees) {
    if (indegree == 0) {
      if (topology.count(member) > 0) {
        string cycleEntrance = find_cycle_entry(topology, member);
        if (cycleEntrance.empty()) continue;

        string prevMember = cycleEntrance;
        string nextMember = topology[prevMember];

        topology[prevMember] = member;
        topology[member] = nextMember;
        cout << Utils::getTime() << "Fixed topology:\n\t" << prevMember
             << "\n\t-> " << member << "\n\t-> " << nextMember << endl;
      }
    }
  }
}

void delete_member(string member) {
  MemberServerContext::RingMapMutex.lock();

  if (MemberServerContext::RingMap.count(member) != 0) {
    MemberServerContext::RingMap.erase(member);
    MemberServerContext::RemovedMember[member] = getNow();
    Member mem = parse_member_id(member);

    cout << Utils::getTime() << "Member [" << mem.hostname << ":" << mem.port
         << "] is deleted from group" << endl;
  }

  MemberServerContext::RingMapMutex.unlock();
}

string get_leader(set<string>& ringMap) { return *ringMap.begin(); }

void update_ringmap(set<string>& incomingRingMap,
                    vector<string>& removedMembers,
                    string failure_detection_message) {
  // MemberUtils::printMemberList("Diff Map", diffMap);

  if ((incomingRingMap.size() != MemberServerContext::RingMap.size()) &&
      !failure_detection_message.empty()) {
    cout << Utils::getTime()
         << "Receive failure detection message: " << failure_detection_message
         << endl;
    MemberServerContext::failureDetectionMessage = failure_detection_message;
  }

  MemberServerContext::RingMapMutex.lock();

  for (auto const member : removedMembers) {
    // add new removed-member to RemovedMember map
    if (MemberServerContext::RemovedMember.count(member) == 0) {
      MemberServerContext::RemovedMember[member] = getNow();
    }

    // remove member from RingMap
    if (MemberServerContext::RingMap.count(member) != 0) {
      MemberServerContext::RingMap.erase(member);
    }
  }

  for (auto member : incomingRingMap) {
    if (MemberServerContext::RemovedMember.count(member) == 0 &&
        MemberServerContext::RingMap.count(member) == 0) {
      MemberServerContext::RingMap.insert(member);
    }
  }

  // update leader
  MemberServerContext::Leader = get_leader(MemberServerContext::RingMap);

  MemberServerContext::RingMapMutex.unlock();
}

void MemberApiHandler::udp_ping_handler(MemberPingRequest* request) {
  string prev_leader = MemberServerContext::Leader;

  update_ringmap(request->ring_topology, request->removed_members,
                 request->failure_detection_message);  // update leader

  // update filesystem status
  FileSystemUtils::update_filesystem_status(
      request->filesystem_sequence_number,
      request->filesystem_file_metadata_map,
      request->filesystem_recovered_failure_servers);

  // update ml status
  MlUtils::update_ml_status(request->ml_job_metadata_map,
                            request->ml_job_status_map);

  // leader needs to update worker_pool
  if (MemberServerContext::SelfMember == MemberServerContext::Leader) {
    // if this member first time become leader: reset worker_pool and
    // all_workers
    if (MemberServerContext::SelfMember != prev_leader) {
      MlUtils::reset_ml_worker_pool();
      MlUtils::handle_running_subjobs();
    }
    MlUtils::update_ml_worker_pool();
  }

  delete request;
}

void MemberApiHandler::udp_receiver(int port) {
  int sockfd;
  string ack = "ACK" + ACK_DELIMITER + Utils::getHostName() + ACK_DELIMITER +
               to_string(port);
  struct sockaddr_in servaddr, cliaddr;

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;          // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;  // The socket accepts connections to
                                          // all the IPs of the machine
  servaddr.sin_port = htons(port);

  // Bind the socket with the server address
  if (::bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  cout << Utils::getTime() << "[UDP] Listening on port " << port << endl;

  while (1) {
    int n, len;
    len = sizeof(cliaddr);
    char buffer[SOCKET_BUFFER_SIZE] = {0};

    n = recvfrom(sockfd, (char*)buffer, SOCKET_BUFFER_SIZE, MSG_WAITALL,
                 (struct sockaddr*)&cliaddr, (socklen_t*)&len);

    // ignore all udp traffic if server mode is waiting introducer
    if (MemberServerContext::ServerMode ==
        MemberServerContext::SERVER_MODE::WAIT_INTRODUCER) {
      continue;
    }

    // response ack message immediately
    sendto(sockfd, (const char*)ack.c_str(), strlen(ack.c_str()), 0,
           (const struct sockaddr*)&cliaddr, len);  // MSG_CONFIRM -> 0

    buffer[n] = '\0';

    MemberPingRequest* request = new MemberPingRequest(buffer);
    thread(udp_ping_handler, request).detach();
  }

  close(sockfd);
}

// create a thread to start receiving ping messages
void MemberApiHandler::start_udp_receiver(int port) {
  thread(&MemberApiHandler::udp_receiver, port).detach();
}

string MemberApiHandler::udp_sender(string ip_address, int port,
                                    SocketMessage* request) {
  int sockfd;
  char buffer[SOCKET_BUFFER_SIZE] = {0};
  struct sockaddr_in servaddr;

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Set response timeout
  struct timeval tv;
  tv.tv_sec = SOCKET_MESSAGE_CONST::UDP_RECEIVE_TIMEOUT_SEC;
  tv.tv_usec = SOCKET_MESSAGE_CONST::UDP_RECEIVE_TIMEOUT_USEC;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    perror("Error");
  }

  memset(&servaddr, 0, sizeof(servaddr));

  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(ip_address.c_str());

  int n, len;
  string request_str = request->serialize();
  MEMBER_TRAFFIC_MONITOR::udp_output_bits += request_str.size() * 8;

  sendto(sockfd, request_str.c_str(), request_str.size(), 0,
         (const struct sockaddr*)&servaddr,
         sizeof(servaddr));  // MSG_CONFIRM -> 0

  n = recvfrom(sockfd, (char*)buffer, SOCKET_BUFFER_SIZE, MSG_WAITALL,
               (struct sockaddr*)&servaddr, (socklen_t*)&len);

  buffer[n] = '\0';
  close(sockfd);
  return string(buffer);
}

// Erase "ACK"
string parse_ack_message(string response) {
  size_t pos = 0;
  if ((pos = response.find(ACK_DELIMITER)) != std::string::npos) {
    response.erase(0, pos + ACK_DELIMITER.length());
  } else {
    throw runtime_error("Error occurs when parsing ACK message");
  }
  return response;
}

void MemberApiHandler::ping_k_member() {
  uint64_t last_ping_time = getNow() / TIME_OFFSET;
  while (1) {
    if (getNow() / TIME_OFFSET - last_ping_time < PERIODIC_PING_CYCLE) {
      // Mac
      // usleep(PERIODIC_PING_CYCLE -
      //        (getNow() / TIME_OFFSET -
      //         last_ping_time)); // microseconds
      // Linux
      usleep((PERIODIC_PING_CYCLE - (getNow() / TIME_OFFSET - last_ping_time)) /
             1000);  // microseconds
    } else {
      last_ping_time = getNow() / TIME_OFFSET;

      // lock ring map
      MemberServerContext::RingMapMutex.lock();

      // get receiver list (toplogy)
      vector<string> ping_members = get_k_successor_member(
          MemberServerContext::SelfMember,
          MemberServerContext::SUCCESSOR_NUMBER);  // replace with member info

      // send diff map to k members
      vector<future<string>> futures;
      vector<thread> thread_pool;

      auto request_handler = [&](promise<string> promise, string host,
                                 string port, SocketMessage* request) {
        try {
          string response =
              (string)MemberApiHandler::udp_sender(host, stoi(port), request);
          delete request;
          promise.set_value(response);
        } catch (exception& e) {
          cout << Utils::getTime() << e.what() << endl;
          promise.set_value(nullptr);
        }
      };

      // cout << Utils::getTime() << "Pinging " << ping_members.size()
      //      << " members" << endl;

      // setup promise, future, and threads for each member
      for (auto member_id : ping_members) {
        try {
          Member ping_member = parse_member_id(member_id);
          promise<string> promise;
          futures.push_back(promise.get_future());
          // cout << Utils::getTime() << "Ping " << ping_member.hostname << ":"
          //      << ping_member.port << endl;

          vector<string> removedMembers;
          for (auto const& [key, value] : MemberServerContext::RemovedMember)
            removedMembers.push_back(key);

          FileSystemServerContext::GlobalMutex.lock();
          MlServerContext::GlobalMutex.lock();
          MlServerContext::LocalMutex.lock();

          MemberPingRequest* request = new MemberPingRequest(
              MemberServerContext::RingMap, removedMembers,
              MemberServerContext::failureDetectionMessage,
              FileSystemServerContext::SequenceNumber,
              FileSystemServerContext::FileMetadataMap,
              FileSystemServerContext::RecoveredFailureServers,
              MlServerContext::JobMetadataMap, MlServerContext::JobStatusMap);

          MlServerContext::LocalMutex.unlock();
          MlServerContext::GlobalMutex.unlock();
          FileSystemServerContext::GlobalMutex.unlock();

          if (!MemberServerContext::failureDetectionMessage.empty()) {
            cout << Utils::getTime() << " Send failure detection message: "
                 << MemberServerContext::failureDetectionMessage << endl;
          }

          thread thread(request_handler, move(promise),
                        HostToIp(ping_member.hostname), ping_member.port,
                        request);
          thread_pool.push_back(move(thread));
        } catch (exception e) {
          cout << Utils::getTime() << e.what() << endl;
        }
      }

      // release lock
      MemberServerContext::RingMapMutex.unlock();

      // start all threads and wait for all
      for (auto&& thread : thread_pool) {
        thread.join();
      }

      int receive_count = 0;
      MemberServerContext::SuccessfulMember.clear();
      MemberServerContext::failureDetectionMessage.clear();
      // aggregate results
      for (auto&& future : futures) {
        string response = future.get();
        if (!response.empty() && response.rfind("ACK", 0) == 0) {
          // Collect successful members
          try {
            string ping_member_location = parse_ack_message(response);
            MemberServerContext::SuspectFailedTime[ping_member_location] = 0;
            MemberServerContext::SuccessfulMember.push_back(
                ping_member_location);
            receive_count++;
          } catch (exception e) {
            cout << Utils::getTime() << e.what() << endl;
          }
        }
      }

      // Set suspect failure time for those did not send back response
      if (receive_count < ping_members.size()) {
        for (auto member_id : ping_members) {
          Member ping_member;
          try {
            ping_member = parse_member_id(member_id);
          } catch (exception e) {
            cerr << e.what() << endl;
            continue;
          }
          string ping_member_location =
              ping_member.hostname + ":" + ping_member.port;
          if (!(find(MemberServerContext::SuccessfulMember.begin(),
                     MemberServerContext::SuccessfulMember.end(),
                     ping_member_location) !=
                MemberServerContext::SuccessfulMember.end())) {
            cout << Utils::getTime()
                 << "Non successful member: " << ping_member_location << endl;
            // First time to be pinged by the sender
            if (MemberServerContext::SuspectFailedTime.find(
                    ping_member_location) ==
                MemberServerContext::SuspectFailedTime.end()) {
              MemberServerContext::SuspectFailedTime[ping_member_location] =
                  getNow() / TIME_OFFSET - last_ping_time;
            } else {
              MemberServerContext::SuspectFailedTime[ping_member_location] +=
                  PERIODIC_PING_CYCLE;
            }
            cout << Utils::getTime() << "Suspect failed time: "
                 << MemberServerContext::SuspectFailedTime[ping_member_location]
                 << endl;

            if (MemberServerContext::SuspectFailedTime[ping_member_location] >=
                MEMBER_CONST::FAILURE_TIMEOUT) {
              Member member = parse_member_id(member_id);
              cout << Utils::getTime() << " Member [" << member.hostname << ":"
                   << member.port << "] is detected failure by ["
                   << MemberServerContext::SelfMember << "]" << endl;
              MemberServerContext::failureDetectionMessage +=
                  Utils::getTime() + " Member [" + member.hostname + ":" +
                  member.port + "] is detected failure by [" +
                  MemberServerContext::SelfMember + "]" + "\n";
              delete_member(member_id);
            }
          }
        }
      }
    }
  }
}

void add_member(string hostname, int port) {
  // get the new member id
  MemberGetIdRequest* getIdRequest = new MemberGetIdRequest();
  MemberGetIdResponse* response;
  try {
    response = (MemberGetIdResponse*)RequestSender::send_request(
        hostname.c_str(), port, getIdRequest);

    if (!response) {
      cout << Utils::getTime() << "Cannot add member : " << hostname << ":"
           << port << endl;
      return;
    }
  } catch (exception e) {
    cerr << e.what() << endl;
    return;
  }

  string newMember = response->memberId;
  delete response;
  delete getIdRequest;

  MemberServerContext::RingMapMutex.lock();
  // Check whether new member is a 'new member'
  if (MemberServerContext::RingMap.count(newMember) > 0) {
    MemberServerContext::RingMapMutex.unlock();
    return;
  }

  // Add new member to the ring
  MemberServerContext::RingMap.insert(newMember);

  MemberSetRingmapRequest* request =
      new MemberSetRingmapRequest(MemberServerContext::RingMap);
  MemberServerContext::RingMapMutex.unlock();

  Member member = parse_member_id(newMember);
  cout << Utils::getTime() << " Member [" << member.hostname << ":" << port
       << "] joins group" << endl;

  try {
    RequestSender::send_request(hostname.c_str(), port, request);
  } catch (exception e) {
    cerr << e.what() << endl;
  }
  delete request;
}

void leave_member_group() {
  // reset context
  MemberServerContext::CreateContext();
  FileSystemServerContext::CreateContext();
}

void set_ring_map(set<string>& ringMap) {
  MemberServerContext::RingMapMutex.lock();

  MemberServerContext::RingMap = ringMap;

  MemberServerContext::RingMapMutex.unlock();
}

SocketMessage* MemberApiHandler::api_handler(const char* buffer) {
  SocketMessage* output = nullptr;
  vector<string> tokens = SocketMessageUtils::parse_tokens(buffer);
  try {
    SOCKET_MESSAGE_TYPE message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
    if (JOIN == message_type) {
      MemberRequest* request = new MemberRequest(buffer);
      // Join the machine into the group
      // if (hostname == MemberServerContext::Introducer) {

      // set introducer itself active member
      MemberServerContext::ServerMode =
          MemberServerContext::SERVER_MODE::ACTIVE_MEMBER;

      add_member(request->hostname, stoi(request->port));
      // }
      // // Send TCP request to introducer
      // else {
      //     output =
      //     RequestSender::send_request(HostToIp(MemberServerContext::Introducer).c_str(),
      //     PORT, request);
      // }
    } else if (LEAVE == message_type) {
      MemberRequest* request = new MemberRequest(buffer);
      leave_member_group();
      cout << Utils::getTime() << " Member [" << request->hostname << ":"
           << request->port << "] leaves group" << endl;
    } else if (MEMBERSHIP == message_type) {
      MemberUtils::printMembershipList();
    } else if (MEMBER_PING == message_type) {
    } else if (MEMBER_GET_ID == message_type) {
      output = new MemberGetIdResponse(MemberServerContext::SelfMember);
    } else if (MEMBER_SET_RINGMAP == message_type) {
      // invited by introducer, set ringmap and server mode
      MemberSetRingmapRequest* request = new MemberSetRingmapRequest(buffer);

      set_ring_map(request->ring_topology);

      MemberServerContext::ServerMode =
          MemberServerContext::SERVER_MODE::ACTIVE_MEMBER;

    } else {
      cout << Utils::getTime() << "Request not found" << endl;
    }
  } catch (invalid_argument& e) {
    cout << Utils::getTime() << "Failed to parse socket request" << endl;
    return new FailedResponse();
  }

  if (output == nullptr) {
    output = new SuccessfulResponse();
  }

  // cout << Utils::getTime() << "Result: " << output << endl;
  return output;
}
