#ifndef FILESYSTEM_LS_RESPONSE
#define FILESYSTEM_LS_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemLsResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::LS;
  string stored_addresses;

  FileSystemLsResponse(){};
  FileSystemLsResponse(const char* byte) { deserialize(byte); }

  FileSystemLsResponse(string stored_addresses);

  virtual ~FileSystemLsResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif