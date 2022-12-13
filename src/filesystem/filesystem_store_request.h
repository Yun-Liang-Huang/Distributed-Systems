#ifndef FILESYSTEM_STORE_REQUEST
#define FILESYSTEM_STORE_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemStoreRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::STORE;

  FileSystemStoreRequest(){};
  FileSystemStoreRequest(const char* byte) { deserialize(byte); }

  virtual ~FileSystemStoreRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif