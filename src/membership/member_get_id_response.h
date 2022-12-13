#ifndef MEMBER_GET_ID_RESPONSE
#define MEMBER_GET_ID_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"

using namespace std;

class MemberGetIdResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::MEMBER_GET_ID;
  string memberId;

  MemberGetIdResponse(){};

  MemberGetIdResponse(string);

  MemberGetIdResponse(const char* byte);

  virtual ~MemberGetIdResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif