#ifndef FILESYSTEM_LS_REQUEST
#define FILESYSTEM_LS_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemLsRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::LS;
  string sdfsfilename;

  FileSystemLsRequest(){};

  FileSystemLsRequest(string sdfsfilename);
  FileSystemLsRequest(const char* byte) { deserialize(byte); }

  virtual ~FileSystemLsRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif