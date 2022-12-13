#ifndef FILESYSTEM_UTILS
#define FILESYSTEM_UTILS

#include <cstddef>  // size_t
#include <fstream>
#include <iostream>

#include "../client_request_sender.h"
#include "../membership/member_server_context.h"
#include "../membership/member_utils.h"
#include "../utils.h"
#include "filesystem_delete_request.h"
#include "filesystem_get_filesize_request.h"
#include "filesystem_get_filesize_response.h"
#include "filesystem_get_request.h"
#include "filesystem_put_request.h"
#include "filesystem_replicate_request.h"
#include "filesystem_replicate_response.h"
#include "filesystem_server_context.h"

using namespace std;

const string VERSION_DELIMITER = "_2oxY7SM9ky";

class FileSystemUtils {
 public:
  static void update_filesystem_status(
      int sequence_number,
      unordered_map<string, FileMetadata*>& file_metadata_map,
      unordered_set<string> filesystem_recovered_failure_servers) {
    FileSystemServerContext::GlobalMutex.lock();

    // leader should never update when receive these status from others
    if (sequence_number > FileSystemServerContext::SequenceNumber) {
      FileSystemServerContext::SequenceNumber = sequence_number;
      FileSystemServerContext::FileMetadataMap =
          file_metadata_map;  // free FileMetadataMap to avoid potential memory
                              // leak
      FileSystemServerContext::RecoveredFailureServers =
          filesystem_recovered_failure_servers;
    }

    FileSystemServerContext::GlobalMutex.unlock();
  }

  static string get_sdfs_filename(string sdfsfilename, int sequence_number) {
    string new_sdfsfilename;
    size_t found = sdfsfilename.find_last_of(".");
    if (found == string::npos) {
      new_sdfsfilename =
          sdfsfilename + "_v" + to_string(sequence_number) + VERSION_DELIMITER;
    } else {
      string filename = sdfsfilename.substr(0, found);
      string type = sdfsfilename.substr(found + 1);

      new_sdfsfilename = filename + "_v" + to_string(sequence_number) +
                         VERSION_DELIMITER + "." + type;
    }

    return new_sdfsfilename;
  }

  static void difference(unordered_set<string>& set1,
                         unordered_set<string>& set2,
                         unordered_set<string>& result) {
    for (auto key : set1) {
      if (set2.count(key) == 0) {
        result.insert(key);
      }
    }
  }

  static void collect_replicating_files(unordered_set<string>& failure_servers,
                                        unordered_set<string>& todo_files) {
    for (auto const& [filename, metadata] :
         FileSystemServerContext::FileMetadataMap) {
      for (auto server : failure_servers) {
        if (metadata->replica_ids.count(server) > 0) {
          todo_files.insert(filename);
          break;
        }
      }
    }
  }

  static void handle_server_failure() {
    if (MemberServerContext::Leader != MemberServerContext::SelfMember) {
      return;
    }

    MemberServerContext::RingMapMutex.lock();
    FileSystemServerContext::GlobalMutex.lock();
    bool is_status_updated = false;

    cout << "=========== [SDFS] ===========" << endl;

    // Step 1: collect failure servers
    unordered_set<string> full_failure_servers;
    unordered_set<string> todo_failure_servers;

    for (auto const& [server_id, _] : MemberServerContext::RemovedMember) {
      full_failure_servers.insert(server_id);
    }

    difference(full_failure_servers,
               FileSystemServerContext::RecoveredFailureServers,
               todo_failure_servers);

    cout << Utils::getTime() << "[SDFS] Collected "
         << todo_failure_servers.size() << " failure servers" << endl;

    // Step 2: collect affected files
    unordered_set<string> todo_replicating_files;
    collect_replicating_files(todo_failure_servers, todo_replicating_files);
    cout << Utils::getTime() << "[SDFS] Collected "
         << todo_replicating_files.size() << " affected files" << endl;

    // Step 3: handle the files one by one
    for (auto filename : todo_replicating_files) {
      cout << Utils::getTime() << "[SDFS] Start replicating file: '" << filename
           << "'" << endl;

      // Step 4: regenerate re-replication list
      unordered_set<string> survived_replicas;
      unordered_set<string> new_replicas;

      difference(
          FileSystemServerContext::FileMetadataMap[filename]->replica_ids,
          todo_failure_servers, survived_replicas);
      MemberUtils::getNRandomMembers(MemberServerContext::RingMap,
                                     FileSystemServerContext::REPLICA_NUMBER,
                                     survived_replicas, new_replicas);
      cout << Utils::getTime() << "[SDFS] Got " << survived_replicas.size()
           << " survived replicas" << endl;

      // Step 5: assign a non-faulty server, send re-replication request for
      // that file

      auto request = new FileSystemReplicateRequest(filename, new_replicas);
      Member replication_worker =
          MemberUtils::parse_member_id(*survived_replicas.begin());

      cout << Utils::getTime() << "[SDFS] Start replicating '" << filename
           << "' with worker: " << replication_worker.hostname << endl;
      auto response =
          RequestSender::send_request(replication_worker.hostname.c_str(),
                                      stoi(replication_worker.port), request);

      // Step 6: commit the file if it is successfully re-replicated (collect
      // result)

      // successful replicated, commit the file into recovered list
      FileSystemServerContext::FileMetadataMap[filename]->replica_ids =
          new_replicas;
      is_status_updated = true;
      cout << Utils::getTime() << "[SDFS] File '" << filename
           << "' replication successfully" << endl;

      // TODO: handle replication timeout cases
    }

    // Step 7: commit recovered servers
    cout << Utils::getTime() << "[SDFS] Committing recovered servers" << endl;
    for (auto server : todo_failure_servers) {
      bool fully_recovered = true;
      for (auto const& [filename, metadata] :
           FileSystemServerContext::FileMetadataMap) {
        if (metadata->replica_ids.count(server) > 0) {
          fully_recovered = false;
          break;
        }
      }

      if (fully_recovered) {
        FileSystemServerContext::RecoveredFailureServers.insert(server);
        is_status_updated = true;
        cout << Utils::getTime()
             << "[SDFS] Recovered replication of failed server '" << server
             << "'" << endl;
      }
    }

    if (is_status_updated) FileSystemServerContext::SequenceNumber++;

    FileSystemServerContext::GlobalMutex.unlock();
    MemberServerContext::RingMapMutex.unlock();
  }

  static int64_t get_replica_filesize(string replica_id, string sdfsfilename,
                                      int version) {
    int64_t filesize = 0;
    Member member = MemberUtils::parse_member_id(replica_id);
    cout << Utils::getTime()
         << "Get filesize from replica id: " << member.hostname << ":"
         << member.port << endl;
    FileSystemGetFilesizeRequest* request =
        new FileSystemGetFilesizeRequest(sdfsfilename, version);
    FileSystemGetFilesizeResponse* response;
    try {
      response = (FileSystemGetFilesizeResponse*)RequestSender::send_request(
          member.hostname.c_str(), stoi(member.port), request);

      if (!response) {
        cout << Utils::getTime() << "Cannot send get filesize: " << sdfsfilename
             << " to replica id: " << member.hostname << ":" << member.port
             << endl;
      }
    } catch (exception e) {
      cerr << e.what() << endl;
    }

    if (GET_FILESIZE == response->message_type) {
      cout << Utils::getTime()
           << "Success to send get filesize: " << sdfsfilename
           << " to replica id: " << member.hostname << ":" << member.port
           << endl;
      filesize = response->filesize;
    } else {
      cout << Utils::getTime() << "Fail to send get filesize: " << sdfsfilename
           << " to replica id: " << member.hostname << ":" << member.port
           << endl;
    }

    return filesize;
  }

  static int64_t get_filesize(string filename) {
    ifstream file(filename, ifstream::ate | ifstream::binary);
    return file.tellg();
  }

  static bool is_file_exist(const char* fileName) {
    ifstream infile(fileName);
    return infile.good();
  }

  /**
   * API for getting file from SDFS (server internal level API)
   */
  static void get_file(string sdfs_filename, string local_filename) {
    FileSystemGetRequest* request =
        new FileSystemGetRequest(sdfs_filename, local_filename, true);

    Member local_server =
        MemberUtils::parse_member_id(MemberServerContext::SelfMember);
    SocketMessage* response = RequestSender::send_request(
        local_server.hostname.c_str(), stoi(local_server.port), request);

    if (!Utils::is_file_exists(local_filename)) {
      cerr << "Failed to get file from SDFS" << endl;
      throw std::runtime_error("Failed to get file from SDFS");
    }
  }

  static string get_file_temp(string sdfs_filename) {
    char temp_local_filename[40];
    tmpnam(temp_local_filename);
    get_file(sdfs_filename, temp_local_filename);

    return temp_local_filename;
  }

  static void put_file(string local_filename, string sdfs_filename) {
    FileSystemPutRequest* request =
        new FileSystemPutRequest(local_filename, sdfs_filename, 0, true);

    Member local_server =
        MemberUtils::parse_member_id(MemberServerContext::SelfMember);
    SocketMessage* response = RequestSender::send_request(
        local_server.hostname.c_str(), stoi(local_server.port), request);

    delete request;
    delete response;
  }

  static void delete_file(string sdfs_filename) {
    FileSystemDeleteRequest* request =
        new FileSystemDeleteRequest(sdfs_filename, true);

    Member local_server =
        MemberUtils::parse_member_id(MemberServerContext::SelfMember);
    SocketMessage* response = RequestSender::send_request(
        local_server.hostname.c_str(), stoi(local_server.port), request);

    delete request;
    delete response;
  }
};

#endif