#ifndef MEMBER_REQUEST
#define MEMBER_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"

using namespace std;

class MemberRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type;
  string hostname;
  string port;

  MemberRequest(){};

  MemberRequest(SOCKET_MESSAGE_TYPE message_type, string hostname, string port);
  MemberRequest(SOCKET_MESSAGE_TYPE message_type);
  MemberRequest(const char* byte) { deserialize(byte); }

  virtual ~MemberRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif