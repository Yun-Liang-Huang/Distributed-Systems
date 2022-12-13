#ifndef MEMBER_GET_ID_REQUEST
#define MEMBER_GET_ID_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"

using namespace std;

class MemberGetIdRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::MEMBER_GET_ID;

  MemberGetIdRequest(){};

  MemberGetIdRequest(const char* byte) { deserialize(byte); }

  virtual ~MemberGetIdRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif