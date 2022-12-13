#ifndef SUCCESSFUL_RESPONSE
#define SUCCESSFUL_RESPONSE

#include <iostream>
#include <string>

#include "socket_message.h"

using namespace std;

class SuccessfulResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::SUCCESSFUL;

  SuccessfulResponse(){};
  SuccessfulResponse(const char* byte) { deserialize(byte); }

  virtual ~SuccessfulResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif