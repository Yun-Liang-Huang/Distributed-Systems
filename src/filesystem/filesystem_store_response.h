#ifndef FILESYSTEM_STORE_RESPONSE
#define FILESYSTEM_STORE_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemStoreResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::STORE;
  string stored_files;

  FileSystemStoreResponse(){};
  FileSystemStoreResponse(const char* byte) { deserialize(byte); }

  FileSystemStoreResponse(string stored_files);

  virtual ~FileSystemStoreResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif