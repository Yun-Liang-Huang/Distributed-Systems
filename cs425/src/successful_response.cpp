#include "successful_response.h"

#include <iostream>
#include <string>
#include <vector>

#include "socket_message_utils.h"

using namespace std;

const string SuccessfulResponse::serialize() { return to_string(message_type); }

void SuccessfulResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
}