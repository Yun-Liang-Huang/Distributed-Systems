#ifndef FILESYSTEM_GET_FILESIZE_REQUEST
#define FILESYSTEM_GET_FILESIZE_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemGetFilesizeRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::GET_FILESIZE;
  string sdfsfilename;
  int version;

  FileSystemGetFilesizeRequest(){};

  FileSystemGetFilesizeRequest(string sdfsfilename, int version);
  FileSystemGetFilesizeRequest(const char* byte) { deserialize(byte); }

  virtual ~FileSystemGetFilesizeRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif