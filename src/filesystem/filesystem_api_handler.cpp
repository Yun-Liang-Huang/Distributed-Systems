#include "filesystem_api_handler.h"

#include <sstream>
#include <unordered_set>

#include "../client_request_sender.h"
#include "../membership/member_server_context.h"
#include "../membership/member_utils.h"
#include "../socket_message.h"
#include "../socket_message_utils.h"
#include "../utils.h"
#include "filesystem_commit_file_request.h"
#include "filesystem_delete_file_metadata_request.h"
#include "filesystem_delete_request.h"
#include "filesystem_file_transfer.h"
#include "filesystem_get_file_metadata_request.h"
#include "filesystem_get_file_metadata_response.h"
#include "filesystem_get_filesize_request.h"
#include "filesystem_get_filesize_response.h"
#include "filesystem_get_request.h"
#include "filesystem_get_versions_request.h"
#include "filesystem_ls_request.h"
#include "filesystem_ls_response.h"
#include "filesystem_put_request.h"
#include "filesystem_replicate_request.h"
#include "filesystem_replicate_response.h"
#include "filesystem_server_context.h"
#include "filesystem_store_request.h"
#include "filesystem_store_response.h"
#include "filesystem_utils.h"

using namespace std;

SocketMessage* get_file_metadata(string filename,
                                 unordered_set<string> base_members) {
  FileSystemServerContext::GlobalMutex.lock();
  // check whether file metadata is existed, if so, get the replicaion list
  if (FileSystemServerContext::FileMetadataMap.count(filename) == 0) {
    // not found, create metadata for the file
    FileSystemServerContext::FileMetadataMap[filename] = new FileMetadata();

    MemberUtils::getNRandomMembers(
        MemberServerContext::RingMap, FileSystemServerContext::REPLICA_NUMBER,
        base_members,
        FileSystemServerContext::FileMetadataMap[filename]->replica_ids);
  }

  unordered_set<string> replicas =
      FileSystemServerContext::FileMetadataMap[filename]->replica_ids;

  // get sequence number
  int seq_number = FileSystemServerContext::SequenceNumber++;

  FileSystemServerContext::GlobalMutex.unlock();

  // return replica list and seq number
  return new FileSystemGetFileMetadataResponse(replicas, seq_number);
}

void commit_file(string filename, int sequence_number) {
  FileSystemServerContext::GlobalMutex.lock();

  if (FileSystemServerContext::FileMetadataMap.count(filename) == 0) {
    FileSystemServerContext::GlobalMutex.unlock();
    throw "Cannot commit file '" + filename + "' due to missing metadata.";
  }

  FileSystemServerContext::FileMetadataMap[filename]->committed_versions.insert(
      sequence_number);

  FileSystemServerContext::SequenceNumber++;

  FileSystemServerContext::GlobalMutex.unlock();
}

SocketMessage* delete_file_metadata(string filename) {
  FileSystemServerContext::GlobalMutex.lock();

  // delete filename from file metadata
  if (FileSystemServerContext::FileMetadataMap.count(filename) != 0) {
    FileSystemServerContext::FileMetadataMap.erase(filename);
  }

  FileSystemServerContext::SequenceNumber++;

  FileSystemServerContext::GlobalMutex.unlock();

  // return successful response
  return new SuccessfulResponse();
}

SocketMessage* get_replica_list(string sdfsfilename) {
  Member leader = MemberUtils::parse_member_id(MemberServerContext::Leader);
  unordered_set<string> base_replicas = {MemberServerContext::SelfMember};

  FileSystemGetFileMetadataRequest* request =
      new FileSystemGetFileMetadataRequest(sdfsfilename, base_replicas);
  FileSystemGetFileMetadataResponse* response = nullptr;
  try {
    response = (FileSystemGetFileMetadataResponse*)RequestSender::send_request(
        leader.hostname.c_str(), stoi(leader.port), request);

    if (!response) {
      cout << Utils::getTime()
           << "Cannot get file metadata from leader: " << leader.hostname << ":"
           << leader.port << endl;
    }
  } catch (exception e) {
    cerr << e.what() << endl;
  }

  return response;
}

int put_replicas(unordered_set<string> replica_ids, string localfilename,
                 string sdfsfilename, int sequence_number) {
  int success_cnt = 0;
  for (auto itr = replica_ids.begin(); itr != replica_ids.end(); itr++) {
    string replica_id = *itr;
    // skip sending replica to the initiator
    if (replica_id == MemberServerContext::SelfMember) {
      success_cnt++;
      continue;
    }
    Member member = MemberUtils::parse_member_id(replica_id);
    cout << Utils::getTime() << "Send file to replica id: " << member.hostname
         << ":" << member.port << endl;
    FileSystemPutRequest* request = new FileSystemPutRequest(
        localfilename, sdfsfilename, sequence_number, false);
    SocketMessage* response;
    try {
      response = RequestSender::send_request(member.hostname.c_str(),
                                             stoi(member.port), request);

      if (!response) {
        cout << Utils::getTime() << "Cannot send file: " << sdfsfilename
             << " to replica id: " << member.hostname << ":" << member.port
             << endl;
      }
    } catch (exception e) {
      cerr << e.what() << endl;
    }

    if (SUCCESSFUL == response->message_type) {
      cout << Utils::getTime() << "Success to send file: " << sdfsfilename
           << " to replica id: " << member.hostname << ":" << member.port
           << endl;
      success_cnt++;
    } else if (FAILED == response->message_type) {
      cout << Utils::getTime() << "Fail to send file: " << sdfsfilename
           << " to replica id: " << member.hostname << ":" << member.port
           << endl;
    }
  }

  return success_cnt;
}

void send_commit(string sdfsfilename, int sequence_number) {
  Member leader = MemberUtils::parse_member_id(MemberServerContext::Leader);
  cout << Utils::getTime()
       << "Send commit file request for file: " << sdfsfilename
       << " to leader: " << leader.hostname << ":" << leader.port << endl;
  FileSystemCommitFileRequest* request =
      new FileSystemCommitFileRequest(sdfsfilename, sequence_number);
  SocketMessage* response;
  try {
    response = RequestSender::send_request(leader.hostname.c_str(),
                                           stoi(leader.port), request);

    if (!response) {
      cout << Utils::getTime()
           << "Cannot get commit file response for file: " << sdfsfilename
           << "from leader : " << leader.hostname << ":" << leader.port << endl;
      return;
    }
  } catch (exception e) {
    cerr << e.what() << endl;
    return;
  }

  SOCKET_MESSAGE_TYPE message_type =
      SocketMessageUtils::get_message_type(response->serialize().c_str());
  if (SUCCESSFUL == message_type) {
    cout << Utils::getTime() << "Success to commit file: " << sdfsfilename
         << " to leader: " << leader.hostname << ":" << leader.port << endl;
  } else if (FAILED == message_type) {
    cout << Utils::getTime() << "Fail to commit file: " << sdfsfilename
         << " to leader: " << leader.hostname << ":" << leader.port << endl;
  }
}

void put_file(string sdfsfilename, int sequence_number, bool is_initiator) {
  cout << Utils::getTime() << "put sdfsfilename: " << sdfsfilename << endl;
  string temp_sdfsfilename =
      FileSystemUtils::get_sdfs_filename(sdfsfilename, sequence_number);
  unordered_set<string> replica_ids;
  // Initiator tasks: get sequence number and replica list from master
  //                  send file to other servers for replica and commit
  FileSystemGetFileMetadataResponse* response = nullptr;
  if (is_initiator) {
    // keep execute until file has committed
    while (1) {
      response =
          (FileSystemGetFileMetadataResponse*)get_replica_list(sdfsfilename);
      replica_ids = response->replica_ids;
      sequence_number = response->sequence_number;

      // send file to other servers to make replicas
      int success_cnt = 0;
      if (response != nullptr) {
        success_cnt = put_replicas(replica_ids, temp_sdfsfilename, sdfsfilename,
                                   sequence_number);
      }

      // commit file to leader
      if (success_cnt == replica_ids.size()) {
        send_commit(sdfsfilename, sequence_number);
        break;
      } else {
        cout << "[SDFS put_file] Put replica failed. Retry put the file again. "
                "filename: "
             << sdfsfilename << endl;
      }
      usleep(1000000);  // microseconds
    }

    // If the initiator is not in the replica list, remove local file
    if (replica_ids.count(MemberServerContext::SelfMember) == 0) {
      if (remove(sdfsfilename.c_str()) == 0) {
        cout << Utils::getTime()
             << "remove local temp file sdfsfilename: " << sdfsfilename
             << " successfully" << endl;
      }
      return;
    }
  }

  FileSystemServerContext::LocalMutex.lock();

  map<int, string> version_map = map<int, string>();
  FileVersionMetadata* metadata = new FileVersionMetadata();
  // If sdfsfilename not exist -> insert
  //    sdfsfilename exists -> update
  if (FileSystemServerContext::FileMap.find(sdfsfilename) !=
      FileSystemServerContext::FileMap.end()) {
    metadata = FileSystemServerContext::FileMap[sdfsfilename];
  }

  // Add delimiter to file name
  string new_sdfsfilename =
      FileSystemUtils::get_sdfs_filename(sdfsfilename, sequence_number);

  // If the temp filename is different from the filename it should be, rename it
  if (temp_sdfsfilename != new_sdfsfilename) {
    rename(temp_sdfsfilename.c_str(), new_sdfsfilename.c_str());
  }

  metadata->version_filenames[sequence_number] = new_sdfsfilename;
  FileSystemServerContext::FileMap[sdfsfilename] = metadata;

  FileSystemServerContext::LocalMutex.unlock();
  cout << Utils::getTime() << "sdfsfilename: " << sdfsfilename
       << " sequence number: " << sequence_number << " (version "
       << version_map.size() << ") created" << endl;
}

void FileSystemApiHandler::get_replica(string replica_id, string sdfsfilename,
                                       int version) {
  Member member = MemberUtils::parse_member_id(replica_id);
  cout << Utils::getTime() << "Get file from replica id: " << member.hostname
       << ":" << member.port << endl;
  FileSystemGetRequest* request =
      new FileSystemGetRequest(sdfsfilename, sdfsfilename, version, false);
  SocketMessage* response;
  try {
    response = RequestSender::send_request(member.hostname.c_str(),
                                           stoi(member.port), request);

    if (!response) {
      cout << Utils::getTime() << "Cannot send get file: " << sdfsfilename
           << " to replica id: " << member.hostname << ":" << member.port
           << endl;
    }
  } catch (exception e) {
    cerr << e.what() << endl;
  }

  SOCKET_MESSAGE_TYPE message_type =
      SocketMessageUtils::get_message_type(response->serialize().c_str());

  if (SUCCESSFUL == message_type) {
    cout << Utils::getTime() << "Success to send get file: " << sdfsfilename
         << " to replica id: " << member.hostname << ":" << member.port << endl;
  } else if (FAILED == message_type) {
    cout << Utils::getTime() << "Fail to send get file: " << sdfsfilename
         << " to replica id: " << member.hostname << ":" << member.port << endl;
  }
}

void FileSystemApiHandler::get_versions_replica(string replica_id,
                                                string sdfsfilename,
                                                int num_versions,
                                                set<int> versions) {
  Member member = MemberUtils::parse_member_id(replica_id);
  cout << Utils::getTime()
       << "Get-version file from replica id: " << member.hostname << ":"
       << member.port << endl;
  FileSystemGetVersionsRequest* request = new FileSystemGetVersionsRequest(
      sdfsfilename, num_versions, sdfsfilename, versions, false);
  SocketMessage* response;
  try {
    response = RequestSender::send_request(member.hostname.c_str(),
                                           stoi(member.port), request);

    if (!response) {
      cout << Utils::getTime()
           << "Cannot send get-version file: " << sdfsfilename
           << " to replica id: " << member.hostname << ":" << member.port
           << endl;
    }
  } catch (exception e) {
    cerr << e.what() << endl;
  }

  SOCKET_MESSAGE_TYPE message_type =
      SocketMessageUtils::get_message_type(response->serialize().c_str());

  if (SUCCESSFUL == message_type) {
    cout << Utils::getTime()
         << "Success to send get-version file: " << sdfsfilename
         << " to replica id: " << member.hostname << ":" << member.port << endl;
  } else if (FAILED == message_type) {
    cout << Utils::getTime()
         << "Fail to send get-version file: " << sdfsfilename
         << " to replica id: " << member.hostname << ":" << member.port << endl;
  }
}

SocketMessage* send_delete_file_metadata(string sdfsfilename) {
  Member leader = MemberUtils::parse_member_id(MemberServerContext::Leader);

  FileSystemDeleteFileMetadataRequest* request =
      new FileSystemDeleteFileMetadataRequest(sdfsfilename);
  SocketMessage* response = nullptr;
  try {
    response = RequestSender::send_request(leader.hostname.c_str(),
                                           stoi(leader.port), request);

    if (!response) {
      cout << Utils::getTime()
           << "Cannot send delete file metadata to leader: " << leader.hostname
           << ":" << leader.port << endl;
    }
  } catch (exception e) {
    cerr << e.what() << endl;
  }

  return response;
}

void delete_replicas(unordered_set<string> replica_ids, string sdfsfilename) {
  for (auto itr = replica_ids.begin(); itr != replica_ids.end(); itr++) {
    string replica_id = *itr;
    // skip sending replica to the initiator
    if (replica_id == MemberServerContext::SelfMember) {
      continue;
    }
    Member member = MemberUtils::parse_member_id(replica_id);
    cout << Utils::getTime()
         << "Send delete file to replica id: " << member.hostname << ":"
         << member.port << endl;
    FileSystemDeleteRequest* request =
        new FileSystemDeleteRequest(sdfsfilename, false);
    SocketMessage* response;
    try {
      response = RequestSender::send_request(member.hostname.c_str(),
                                             stoi(member.port), request);

      if (!response) {
        cout << Utils::getTime() << "Cannot send delete file: " << sdfsfilename
             << " to replica id: " << member.hostname << ":" << member.port
             << endl;
      }
    } catch (exception e) {
      cerr << e.what() << endl;
    }

    SOCKET_MESSAGE_TYPE message_type =
        SocketMessageUtils::get_message_type(response->serialize().c_str());
    if (SUCCESSFUL == message_type) {
      cout << Utils::getTime()
           << "Success to send delete file: " << sdfsfilename
           << " to replica id: " << member.hostname << ":" << member.port
           << endl;
    } else if (FAILED == message_type) {
      cout << Utils::getTime() << "Fail to send delete file: " << sdfsfilename
           << " to replica id: " << member.hostname << ":" << member.port
           << endl;
    }
  }
}

void delete_file(string sdfsfilename, bool is_initiator) {
  cout << Utils::getTime() << "delete sdfsfilename: " << sdfsfilename << endl;

  if (FileSystemServerContext::FileMetadataMap.count(sdfsfilename) == 0) {
    cout << Utils::getTime() << "sdfsfilename: " << sdfsfilename
         << " does not exist" << endl;
    return;
  }

  // Initiator task: call master to delete sdfsfilename from FileMetadataMap
  //                 send delete request to other servers with replica and
  //                 commit
  unordered_set<string> replica_ids;
  SocketMessage* response;
  if (is_initiator) {
    // cout << Utils::getTime() << "This is initiator" << endl;
    replica_ids =
        FileSystemServerContext::FileMetadataMap[sdfsfilename]->replica_ids;
    response = send_delete_file_metadata(sdfsfilename);

    // send delete request to other servers to delete replicas
    if (response != nullptr) {
      delete_replicas(replica_ids, sdfsfilename);
    }

    // If the initiator is not in the replica list, no need to remove FileMap or
    // local file
    if (replica_ids.find(MemberServerContext::SelfMember) ==
        replica_ids.end()) {
      return;
    }
  }

  FileSystemServerContext::LocalMutex.lock();

  // remove all versions of sdfsfilename on server
  map<int, string> version_map =
      FileSystemServerContext::FileMap[sdfsfilename]->version_filenames;
  for (auto itr = version_map.begin(); itr != version_map.end(); itr++) {
    if (remove(itr->second.c_str()) == 0) {
      cout << Utils::getTime() << "delete file: " << itr->first
           << " successfully" << endl;
    } else {
      perror("Error deleting file ");
    }
  }

  // remove sdfsfilename from server FileMap
  FileSystemServerContext::FileMap.erase(sdfsfilename);

  FileSystemServerContext::LocalMutex.unlock();
}

string get_stored_files() {
  cout << Utils::getTime() << "get stored files" << endl;
  ostringstream oss;
  for (auto itr = FileSystemServerContext::FileMap.begin();
       itr != FileSystemServerContext::FileMap.end(); ++itr) {
    oss << "\t" << itr->first << " ";
  }

  if (oss.str().length() == 0) {
    oss << "no files stored on this machine";
  }
  cout << Utils::getTime() << "get stored files: " << oss.str() << endl;
  return oss.str();
}

string get_stored_addresses(string sdfsfilename) {
  cout << Utils::getTime()
       << "get stored addresses of sdfsfilename: " << sdfsfilename << endl;
  ostringstream oss;
  if (FileSystemServerContext::FileMetadataMap.find(sdfsfilename) !=
      FileSystemServerContext::FileMetadataMap.end()) {
    unordered_set<string> replica_ids =
        FileSystemServerContext::FileMetadataMap[sdfsfilename]->replica_ids;
    for (auto itr = replica_ids.begin(); itr != replica_ids.end(); ++itr) {
      oss << "\t" << *itr << "\n";
    }
  } else {
    oss << "sdfsfilename: " << sdfsfilename << " is not found";
  }
  cout << Utils::getTime() << "get stored addresses: " << oss.str() << endl;
  return oss.str();
}

SocketMessage* replicate_file(string sdfsfilename,
                              unordered_set<string> replica_ids) {
  unordered_set<int> replicated_versions;
  unordered_set<int> committed_versions;
  // get all committed versions
  if (FileSystemServerContext::FileMetadataMap.find(sdfsfilename) !=
      FileSystemServerContext::FileMetadataMap.end()) {
    committed_versions = FileSystemServerContext::FileMetadataMap[sdfsfilename]
                             ->committed_versions;
  }
  // look up sdfsfilename in FileMap and send all committed versions one-by-one
  // to new replica ids
  if (FileSystemServerContext::FileMap.find(sdfsfilename) !=
      FileSystemServerContext::FileMap.end()) {
    map<int, string> version_map =
        FileSystemServerContext::FileMap[sdfsfilename]->version_filenames;
    for (auto itr = version_map.rbegin(); itr != version_map.rend(); itr++) {
      if (committed_versions.find(itr->first) != committed_versions.end()) {
        cout << Utils::getTime() << "replicate file: " << sdfsfilename
             << " version: " << itr->first << endl;
        string new_sdfsfilename =
            FileSystemUtils::get_sdfs_filename(sdfsfilename, itr->first);
        int success_cnt = put_replicas(replica_ids, new_sdfsfilename,
                                       sdfsfilename, itr->first);
        if (success_cnt == replica_ids.size()) {
          replicated_versions.insert(itr->first);
        }
      }
    }
  }

  // commit the replication process
  return new FileSystemReplicateResponse(sdfsfilename, replicated_versions);
}

SocketMessage* get_filesize(string sdfsfilename, int version) {
  int64_t filesize = 0;
  string new_sdfsfilename =
      FileSystemUtils::get_sdfs_filename(sdfsfilename, version);
  filesize = FileSystemUtils::get_filesize(new_sdfsfilename);
  return new FileSystemGetFilesizeResponse(filesize);
}

SocketMessage* FileSystemApiHandler::api_handler(const char* buffer) {
  SocketMessage* output = nullptr;
  vector<string> tokens = SocketMessageUtils::parse_tokens(buffer);

  try {
    SOCKET_MESSAGE_TYPE message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
    if (PUT == message_type) {
      // cout << Utils::getTime() << "PUT args: " << tokens[1] << " " <<
      // tokens[2]
      //      << " " << tokens[3] << " " << tokens[4] << endl;
      put_file(tokens[2], stoi(tokens[3]), stoi(tokens[4]));

    } else if (GET == message_type) {
      // cout << Utils::getTime() << "GET args: " << tokens[1] << " " <<
      // tokens[2]
      //      << " " << tokens[3] << " " << tokens[4] << endl;
    } else if (DELETE == message_type) {
      // cout << Utils::getTime() << "DELETE args: " << tokens[1] << " "
      //      << tokens[2] << endl;
      delete_file(tokens[1], stoi(tokens[2]));

    } else if (LS == message_type) {
      // cout << Utils::getTime() << "LS args: " << tokens[1] << endl;
      string stored_addresses = get_stored_addresses(tokens[1]);
      output = new FileSystemLsResponse(stored_addresses);

    } else if (STORE == message_type) {
      // cout << Utils::getTime() << "STORE args: " << endl;
      string stored_files = get_stored_files();
      output = new FileSystemStoreResponse(stored_files);

    } else if (GET_VERSIONS == message_type) {
      // cout << Utils::getTime() << "GET_VERSIONS args: " << tokens[1] << " "
      //      << tokens[2] << " " << tokens[3] << " " << tokens[4] << endl;
    } else if (GET_FILE_METADATA == message_type) {
      // cout << Utils::getTime() << "GET_FILE_METADATA args: " << tokens[1]
      //      << endl;
      FileSystemGetFileMetadataRequest* request =
          new FileSystemGetFileMetadataRequest(buffer);
      output =
          get_file_metadata(request->sdfs_filename, request->base_replica_ids);
      delete request;
    } else if (COMMIT_FILE == message_type) {
      // cout << Utils::getTime() << "COMMIT_FILE args: " << tokens[1] << endl;

      FileSystemCommitFileRequest* request =
          new FileSystemCommitFileRequest(buffer);
      commit_file(request->sdfs_filename, request->sequence_number);
      delete request;
    } else if (DELETE_FILE_METADATA == message_type) {
      // cout << Utils::getTime() << "DELETE_FILE_METADATA args: " << tokens[1]
      //      << endl;

      FileSystemDeleteFileMetadataRequest* request =
          new FileSystemDeleteFileMetadataRequest(buffer);
      output = delete_file_metadata(request->sdfs_filename);
      delete request;
    } else if (REPLICATE == message_type) {
      // cout << Utils::getTime() << "REPLICATE args: " << tokens[1] << " "
      //      << tokens[2] << endl;

      FileSystemReplicateRequest* request =
          new FileSystemReplicateRequest(buffer);
      output = replicate_file(request->sdfsfilename, request->replica_ids);
      delete request;
    } else if (GET_FILESIZE == message_type) {
      // cout << Utils::getTime() << "GET_FILESIZE args: " << tokens[1] << " "
      //      << tokens[2] << endl;

      FileSystemGetFilesizeRequest* request =
          new FileSystemGetFilesizeRequest(buffer);
      output = get_filesize(request->sdfsfilename, request->version);
      delete request;
    } else {
      cout << Utils::getTime() << "Request not found" << endl;
    }
  } catch (invalid_argument& e) {
    cout << Utils::getTime() << "Failed to parse socket request: " << e.what()
         << endl;
    return new FailedResponse();
  }

  if (output == nullptr) {
    output = new SuccessfulResponse();
  }

  cout << Utils::getTime() << "Result: " << output << endl;
  return output;
}
