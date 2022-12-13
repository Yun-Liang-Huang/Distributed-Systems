#ifndef FILESYSTEM_GET_FILESIZE_RESPONSE
#define FILESYSTEM_GET_FILESIZE_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemGetFilesizeResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::GET_FILESIZE;
  int64_t filesize;

  FileSystemGetFilesizeResponse(){};

  FileSystemGetFilesizeResponse(int64_t filesize);
  FileSystemGetFilesizeResponse(const char* byte) { deserialize(byte); }

  virtual ~FileSystemGetFilesizeResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif