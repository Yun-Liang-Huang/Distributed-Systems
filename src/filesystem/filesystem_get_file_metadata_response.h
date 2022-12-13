#ifndef FILESYSTEM_GET_FILE_METADATA_RESPONSE
#define FILESYSTEM_GET_FILE_METADATA_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemGetFileMetadataResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::GET_FILE_METADATA;
  unordered_set<string> replica_ids;
  int sequence_number;

  FileSystemGetFileMetadataResponse(){};
  FileSystemGetFileMetadataResponse(const char* byte) { deserialize(byte); }

  FileSystemGetFileMetadataResponse(unordered_set<string>, int);

  virtual ~FileSystemGetFileMetadataResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif