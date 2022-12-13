#include "filesystem_file_transfer.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include "../membership/member_server_context.h"
#include "../socket_message.h"
#include "../socket_message_const.h"
#include "../socket_message_utils.h"
#include "../utils.h"
#include "filesystem_api_handler.h"
#include "filesystem_server_context.h"
#include "filesystem_utils.h"

using namespace std;

#define PORT 8088

// Reference:
// https://stackoverflow.com/questions/63494014/sending-files-over-tcp-sockets-c-windows
int64_t FileSystemFileTransfer::send_file(int sockfd, string filename) {
  int64_t fileSize;
  char buffer[SOCKET_BUFFER_SIZE] = {0};
  bool errored = false;
  std::ifstream file(filename, std::ifstream::binary);
  if (file.fail()) {
    cout << Utils::getTime() << "file: " << filename << " cannot be opened"
         << endl;
    return -1;
  }

  // get file size
  fileSize = FileSystemUtils::get_filesize(filename);

  // send file size first
  const char* file_size_str = to_string(fileSize).c_str();
  send(sockfd, file_size_str, strlen(file_size_str), 0);
  read(sockfd, buffer,
       SOCKET_BUFFER_SIZE);  // receive message that the receiver got file size

  cout << Utils::getTime() << "Start sending file: " << filename
       << ", file size: " << fileSize << endl;

  int64_t i = fileSize;
  while (i > 0) {
    const int64_t ssize = min(i, (int64_t)SOCKET_BUFFER_SIZE);
    file.read(buffer, ssize);
    int n = send(sockfd, buffer, ssize, MSG_NOSIGNAL);
    if (!file || n < 0 || n < ssize) {
      errored = true;
      break;
    }
    // cout << Utils::getTime() << "Sending data size: " << n << endl;
    i -= n;
  }

  file.close();

  if (errored) {
    throw runtime_error("Broken pipe");
  }
  
  return errored ? -2 : fileSize;
}

int64_t FileSystemFileTransfer::recv_file(int sockfd, string filename) {
  int64_t fileSize;
  char buffer[SOCKET_BUFFER_SIZE] = {0};
  bool errored = false;
  std::ofstream file(filename, std::ofstream::binary);
  if (file.fail()) {
    cout << Utils::getTime() << "file: " << filename << " cannot be created"
         << endl;
    return -1;
  }

  // get file size first
  recv(sockfd, buffer, SOCKET_BUFFER_SIZE, 0);
  stringstream strstream(buffer);
  strstream >> fileSize;
  send(sockfd, FILE_TRANSFER_MSG.c_str(), sizeof(FILE_TRANSFER_MSG),
       0);  // tell the sender that the receiver got file size
  int64_t i = 0;

  cout << Utils::getTime() << "Start receiving file: " << filename
       << ", file size: " << fileSize << endl;

  while (i < fileSize) {
    int n = recv(sockfd, buffer, SOCKET_BUFFER_SIZE, 0);
    if (n == 0) break;
    if (n < 0 || !file.write(buffer, n)) {
      errored = true;
      break;
    }
    i += n;
    // cout << "Receive file size so far: " << i << endl;
  }

  file.close();
  return errored ? -2 : i;
}

void FileSystemFileTransfer::client_file_transfer(int sockfd,
                                                  string byte_request) {
  char buffer[SOCKET_BUFFER_SIZE] = {0};
  vector<string> tokens =
      SocketMessageUtils::parse_tokens(byte_request.c_str());
  SOCKET_MESSAGE_TYPE message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);

  // client send file to server
  if (PUT == message_type) {
    string localfilename = tokens[1];
    recv(sockfd, buffer, SOCKET_BUFFER_SIZE, 0);
    string resp;
    stringstream strstream(buffer);
    strstream >> resp;
    if (START_RECV_FILE == resp) {
      FileSystemFileTransfer::send_file(sockfd, localfilename);
    }
  }

  // client receive file from server
  else if (GET == message_type) {
    string sdfsfilename = tokens[1];
    string localfilename = tokens[2];
    int version = stoi(tokens[3]);
    bool is_initiator = stoi(tokens[4]);
    recv(sockfd, buffer, SOCKET_BUFFER_SIZE, 0);
    string resp;
    stringstream strstream(buffer);
    strstream >> resp;
    if (START_SEND_FILE == resp) {
      send(sockfd, FILE_TRANSFER_MSG.c_str(), sizeof(FILE_TRANSFER_MSG),
           0);  // tell the sender that the receiver is ready
      string new_localfilename = localfilename;
      // if the server receiving the GET command does not have replica, create a
      // temp file
      if (!is_initiator) {
        new_localfilename =
            FileSystemUtils::get_sdfs_filename(sdfsfilename, version);
      }
      FileSystemFileTransfer::recv_file(sockfd, new_localfilename);
      send(sockfd, FILE_TRANSFER_MSG.c_str(), sizeof(FILE_TRANSFER_MSG),
           0);  // tell the sender that the receiver is done
    } else if (FILE_NOT_EXIST == resp) {
      cout << Utils::getTime() << "sdfsfilename: " << sdfsfilename
           << " does not exist " << endl;
      send(sockfd, FILE_TRANSFER_MSG.c_str(), sizeof(FILE_TRANSFER_MSG), 0);
      return;
    }
  }

  // client receive version files from server
  else if (GET_VERSIONS == message_type) {
    string sdfsfilename = tokens[1];
    int num_versions = stoi(tokens[2]);
    string localfilename = tokens[3];
    set<int> versions;
    SocketMessage::deserialize_set(tokens[4], versions);
    bool is_initiator = stoi(tokens[5]);
    int i = 1;
    set<int>::reverse_iterator itr;
    if (!is_initiator) {
      itr = versions.rbegin();
    }
    while (i <= num_versions) {
      recv(sockfd, buffer, SOCKET_BUFFER_SIZE, 0);
      string resp;
      stringstream strstream(buffer);
      strstream >> resp;
      if (START_SEND_FILE == resp) {
        send(sockfd, FILE_TRANSFER_MSG.c_str(), sizeof(FILE_TRANSFER_MSG),
             0);  // tell the sender that the receiver is ready
        size_t found = localfilename.find_last_of(".");
        string filename = localfilename.substr(0, found);
        string type = localfilename.substr(found + 1);
        string new_localfilename = filename + "_" + to_string(i) + "." + type;
        if (!is_initiator) {
          new_localfilename = FileSystemUtils::get_sdfs_filename(sdfsfilename, *itr);
        }
        FileSystemFileTransfer::recv_file(sockfd, new_localfilename);
        send(sockfd, FILE_TRANSFER_MSG.c_str(), sizeof(FILE_TRANSFER_MSG),
             0);  // tell the sender that the receiver is done
        i++;
        if (!is_initiator && itr != versions.rend()) {
          itr++;
        }
      } else if (GET_VERSIONS_END == resp) {  // the server has sent all version
                                              // files (maybe < num_versions)
        cout << Utils::getTime() << "sdfsfilename: " << sdfsfilename
             << " has only " << i - 1 << " versions in sdfs" << endl;
        return;
      } else if (FILE_NOT_EXIST == resp) {
        cout << Utils::getTime() << "sdfsfilename: " << sdfsfilename
             << " does not exist " << endl;
        send(sockfd, FILE_TRANSFER_MSG.c_str(), sizeof(FILE_TRANSFER_MSG), 0);
        return;
      }
    }
  }
}

void FileSystemFileTransfer::server_file_transfer(int sockfd,
                                                  string byte_request) {
  char buffer[SOCKET_BUFFER_SIZE] = {0};
  vector<string> tokens =
      SocketMessageUtils::parse_tokens(byte_request.c_str());
  SOCKET_MESSAGE_TYPE message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);

  // server receive file from client
  if (PUT == message_type) {
    string sdfsfilename = tokens[2];
    int version = stoi(tokens[3]);
    string new_sdfsfilename = FileSystemUtils::get_sdfs_filename(sdfsfilename, version);
    send(sockfd, START_RECV_FILE.c_str(), sizeof(START_RECV_FILE), 0);
    FileSystemFileTransfer::recv_file(sockfd, new_sdfsfilename);
  }

  // server send file to client
  else if (GET == message_type) {
    string sdfsfilename = tokens[1];
    int version = stoi(tokens[3]);
    bool is_initiator = stoi(tokens[4]);
    bool removed_flag = false;

    // get the latest written and acknowledged version
    if (is_initiator) {
      if (FileSystemServerContext::FileMetadataMap.find(sdfsfilename) ==
          FileSystemServerContext::FileMetadataMap.end()) {
        cout << Utils::getTime() << "sdfsfilename: " << sdfsfilename
             << " does not exist" << endl;
        send(sockfd, FILE_NOT_EXIST.c_str(), sizeof(FILE_NOT_EXIST), 0);
        read(sockfd, buffer, SOCKET_BUFFER_SIZE);
        return;
      } else {
        // check FileMetadataMap to find the latest committed version
        unordered_set<string> replica_ids =
            FileSystemServerContext::FileMetadataMap[sdfsfilename]->replica_ids;
        unordered_set<int> committed_versions =
            FileSystemServerContext::FileMetadataMap[sdfsfilename]
                ->committed_versions;
        for (auto itr = committed_versions.begin();
             itr != committed_versions.end(); itr++) {
          if (*itr > version) {
            version = *itr;
          }
        }

        // Quorum read: get file sizes from R = 3 replica servers
        map<int64_t, string> filesize_map;
        for (auto itr = replica_ids.begin(); itr != replica_ids.end(); itr++) {
          int64_t filesize = FileSystemUtils::get_replica_filesize(
              *itr, sdfsfilename, version);
          filesize_map[filesize] = *itr;
          cout << "filesize: " << filesize << " replica server: " << *itr
               << endl;
        }

        // assume the replica server with max file size is the committed version
        string replica_id = filesize_map.rbegin()->second;

        // the server receiving the GET command does not have a replica,
        // need to get replica from a replica server
        if (replica_id != MemberServerContext::SelfMember &&
            replica_ids.count(MemberServerContext::SelfMember) == 0) {
          removed_flag = true;
          FileSystemApiHandler::get_replica(replica_id, sdfsfilename, version);
        }
      }
    }

    string new_sdfsfilename =
        FileSystemUtils::get_sdfs_filename(sdfsfilename, version);

    // Initiator: send the latest version back to client
    // Non-initiator: send the latest version to the initiator
    send(sockfd, START_SEND_FILE.c_str(), sizeof(START_SEND_FILE), 0);
    read(sockfd, buffer,
         SOCKET_BUFFER_SIZE);  // receive message from client, so that the
                               // server can send file size
    FileSystemFileTransfer::send_file(sockfd, new_sdfsfilename);
    read(sockfd, buffer,
         SOCKET_BUFFER_SIZE);  // receive message from client, saying that
                               // client has received the file

    if (removed_flag) {
      if (remove(new_sdfsfilename.c_str()) == 0) {
        cout << Utils::getTime()
             << "remove local temp file sdfsfilename: " << new_sdfsfilename
             << " successfully" << endl;
      }
    }
  }

  // server send version files to client
  else if (GET_VERSIONS == message_type) {
    string sdfsfilename = tokens[1];
    int num_versions = stoi(tokens[2]);
    set<int> versions;
    SocketMessage::deserialize_set(tokens[4], versions);
    bool is_initiator = stoi(tokens[5]);
    bool removed_flag = false;

    // get the latest written and acknowledged versions
    if (is_initiator) {
      if (FileSystemServerContext::FileMetadataMap.find(sdfsfilename) ==
          FileSystemServerContext::FileMetadataMap.end()) {
        cout << Utils::getTime() << "sdfsfilename: " << sdfsfilename
             << " does not exist" << endl;
        send(sockfd, FILE_NOT_EXIST.c_str(), sizeof(FILE_NOT_EXIST), 0);
        read(sockfd, buffer, SOCKET_BUFFER_SIZE);
        return;
      } else {
        // check FileMetadataMap to find the latest committed versions
        unordered_set<string> replica_ids =
            FileSystemServerContext::FileMetadataMap[sdfsfilename]->replica_ids;
        unordered_set<int> committed_versions =
            FileSystemServerContext::FileMetadataMap[sdfsfilename]
                ->committed_versions;
        set<int> ordered_committed_versions;
        for (auto itr = committed_versions.begin();
             itr != committed_versions.end(); itr++) {
          ordered_committed_versions.insert(*itr);
        }
        int i = 0;
        for (auto itr = ordered_committed_versions.rbegin();
             itr != ordered_committed_versions.rend(); itr++) {
          if (i >= num_versions) {
            break;
          }
          versions.insert(*itr);
          i++;
        }

        // Quorum read: get file sizes from R = 3 replica servers
        map<int64_t, string> filesize_map;
        for (auto itr = replica_ids.begin(); itr != replica_ids.end(); itr++) {
          // get file size from the oldest version (the last one to be
          // replicated)
          int64_t filesize = FileSystemUtils::get_replica_filesize(
              *itr, sdfsfilename, *versions.begin());
          filesize_map[filesize] = *itr;
          cout << Utils::getTime() << "filesize: " << filesize
               << " replica server: " << *itr << endl;
        }

        // assume the replica server with max file size is the committed version
        string replica_id = filesize_map.rbegin()->second;

        if (replica_id != MemberServerContext::SelfMember &&
            replica_ids.count(MemberServerContext::SelfMember) == 0) {
          removed_flag = true;
          // get versions of sdfsfilename
          FileSystemApiHandler::get_versions_replica(replica_id, sdfsfilename,
                                                     num_versions, versions);
        }
      }
    }

    // Initiator: send the latest versions back to client
    // Non-initiator: send the latest versions to the initiator
    int i = num_versions;
    for (auto itr = versions.rbegin(); itr != versions.rend(); ++itr) {
      if (i <= 0) {
        break;  // already send num_versions files
      }
      cout << Utils::getTime() << "version num: " << *itr << endl;
      string new_sdfsfilename =
          FileSystemUtils::get_sdfs_filename(sdfsfilename, *itr);

      send(sockfd, START_SEND_FILE.c_str(), sizeof(START_SEND_FILE), 0);
      read(sockfd, buffer,
           SOCKET_BUFFER_SIZE);  // receive message from client, so that the
                                 // server can send file size
      FileSystemFileTransfer::send_file(sockfd, new_sdfsfilename);
      read(sockfd, buffer,
           SOCKET_BUFFER_SIZE);  // receive message from client, saying that
                                 // client has received the file

      if (removed_flag) {
        if (remove(new_sdfsfilename.c_str()) == 0) {
          cout << Utils::getTime()
               << "remove local temp file sdfsfilename: " << new_sdfsfilename
               << " successfully" << endl;
        }
      }

      i--;
    }
    // sdfsfilename has less than num_versions files
    if (i > 0) {
      send(sockfd, GET_VERSIONS_END.c_str(), sizeof(GET_VERSIONS_END),
           0);  // tell the client all versions are sent
    }
  }
}
