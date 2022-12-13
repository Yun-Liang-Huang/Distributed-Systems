#include "member_get_id_request.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../socket_message.h"
#include "../socket_message_const.h"
#include "../socket_message_utils.h"

using namespace std;

const string MemberGetIdRequest::serialize() { return to_string(message_type); }

void MemberGetIdRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
}