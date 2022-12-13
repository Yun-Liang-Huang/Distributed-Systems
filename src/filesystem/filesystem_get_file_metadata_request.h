#ifndef FILESYSTEM_GET_FILE_METADATA_REQUEST
#define FILESYSTEM_GET_FILE_METADATA_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemGetFileMetadataRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::GET_FILE_METADATA;
  string sdfs_filename;
  unordered_set<string>
      base_replica_ids;  // caller could specify the file's replicas, if the
                         // number of given replicas does not meet system
                         // requirement, leader will stuff the replica list with
                         // other memebers

  FileSystemGetFileMetadataRequest(){};
  FileSystemGetFileMetadataRequest(const char* byte) { deserialize(byte); }

  FileSystemGetFileMetadataRequest(string sdfs_filename);
  FileSystemGetFileMetadataRequest(string sdfs_filename,
                                   unordered_set<string> base_replica_ids);

  virtual ~FileSystemGetFileMetadataRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif