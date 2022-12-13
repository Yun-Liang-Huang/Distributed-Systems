#ifndef FAILED_RESPONSE
#define FAILED_RESPONSE

#include <iostream>
#include <string>

#include "socket_message.h"

using namespace std;

class FailedResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::FAILED;

  FailedResponse(){};
  FailedResponse(const char* byte) { deserialize(byte); }

  virtual ~FailedResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif