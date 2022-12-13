#ifndef MEMBER_SET_RINGMAP_REQUEST
#define MEMBER_SET_RINGMAP_REQUEST

#include <iostream>
#include <string>
#include <unordered_map>

#include "../socket_message.h"

using namespace std;

class MemberSetRingmapRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::MEMBER_SET_RINGMAP;
  set<string> ring_topology;

  MemberSetRingmapRequest(){};

  MemberSetRingmapRequest(
      set<string> ring_topology);  // this will shallow copy the inputted map
  MemberSetRingmapRequest(const char* byte) { deserialize(byte); }

  virtual ~MemberSetRingmapRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif