#include "member_get_id_response.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../socket_message.h"
#include "../socket_message_const.h"
#include "../socket_message_utils.h"

using namespace std;

MemberGetIdResponse::MemberGetIdResponse(string memberId)
    : memberId(memberId) {}

MemberGetIdResponse::MemberGetIdResponse(const char* byte) {
  deserialize(byte);
}

const string MemberGetIdResponse::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + memberId;
}

void MemberGetIdResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  memberId = tokens[1];
}