#include "member_set_ringmap_request.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../socket_message.h"
#include "../socket_message_const.h"
#include "../socket_message_utils.h"

using namespace std;

MemberSetRingmapRequest::MemberSetRingmapRequest(set<string> ring_topology)
    : ring_topology(ring_topology) {}

const string MemberSetRingmapRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER +
         serialize_set(ring_topology);
}

void MemberSetRingmapRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  deserialize_set(tokens[1], ring_topology);
}