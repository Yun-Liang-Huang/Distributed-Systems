#ifndef FILESYSTEM_DELETE_REQUEST
#define FILESYSTEM_DELETE_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemDeleteRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::DELETE;
  string sdfsfilename;
  bool is_initiator;

  FileSystemDeleteRequest(){};

  FileSystemDeleteRequest(string sdfsfilename, bool is_initiator);
  FileSystemDeleteRequest(const char* byte) { deserialize(byte); }

  virtual ~FileSystemDeleteRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif