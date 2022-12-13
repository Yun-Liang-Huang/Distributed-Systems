#include "member_request.h"

#include <string>
#include <vector>

#include "../socket_message.h"
#include "../socket_message_const.h"
#include "../socket_message_utils.h"

using namespace std;

MemberRequest::MemberRequest(SOCKET_MESSAGE_TYPE message_type, string hostname,
                             string port)
    : message_type(message_type), hostname(hostname), port(port) {}

MemberRequest::MemberRequest(SOCKET_MESSAGE_TYPE message_type)
    : message_type(message_type) {}

const string MemberRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + hostname +
         SOCKET_MESSAGE_DELIMITER + port;
}

void MemberRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  hostname = tokens[1];
  port = tokens[2];
}