#ifndef FILESYSTEM_COMMIT_FILE_REQUEST
#define FILESYSTEM_COMMIT_FILE_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemCommitFileRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::COMMIT_FILE;
  string sdfs_filename;
  int sequence_number;

  FileSystemCommitFileRequest(){};
  FileSystemCommitFileRequest(const char* byte) { deserialize(byte); }

  FileSystemCommitFileRequest(string sdfs_filename, int sequence_number);

  virtual ~FileSystemCommitFileRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif