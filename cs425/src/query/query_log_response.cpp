#include "query_log_response.h"

#include <iostream>
#include <string>
#include <vector>

#include "../socket_message.h"
#include "../socket_message_const.h"
#include "../socket_message_utils.h"

using namespace std;

QueryLogResponse::QueryLogResponse(string logs, int match_line_count,
                                   int total_line_count)
    : logs(logs),
      match_line_count(match_line_count),
      total_line_count(total_line_count) {}

QueryLogResponse::QueryLogResponse(const char* byte) { deserialize(byte); }

const string QueryLogResponse::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + logs +
         SOCKET_MESSAGE_DELIMITER + to_string(match_line_count) +
         SOCKET_MESSAGE_DELIMITER + to_string(total_line_count);
}

void QueryLogResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  logs = tokens[1];
  match_line_count = stoi(tokens[2]);
  total_line_count = stoi(tokens[3]);
}