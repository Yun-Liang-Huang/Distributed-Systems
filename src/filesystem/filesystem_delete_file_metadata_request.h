#ifndef FILESYSTEM_DELETE_FILE_METADATA_REQUEST
#define FILESYSTEM_DELETE_FILE_METADATA_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemDeleteFileMetadataRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::DELETE_FILE_METADATA;
  string sdfs_filename;

  FileSystemDeleteFileMetadataRequest(){};
  FileSystemDeleteFileMetadataRequest(const char* byte) { deserialize(byte); }

  FileSystemDeleteFileMetadataRequest(string sdfs_filename);

  virtual ~FileSystemDeleteFileMetadataRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif