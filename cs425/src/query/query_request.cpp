#include "query_request.h"

#include <string>
#include <vector>

#include "../socket_message.h"
#include "../socket_message_const.h"
#include "../socket_message_utils.h"

using namespace std;

QueryRequest::QueryRequest(SOCKET_MESSAGE_TYPE message_type,
                           QUERY_TYPE query_type, string input,
                           string file_pattern)
    : message_type(message_type),
      query_type(query_type),
      input(input),
      file_pattern(file_pattern) {}

const string QueryRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER +
         to_string(query_type) + SOCKET_MESSAGE_DELIMITER + input +
         SOCKET_MESSAGE_DELIMITER + file_pattern;
}

void QueryRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  query_type = (QUERY_TYPE)stoi(tokens[1]);
  input = tokens[2];
  file_pattern = tokens[3];
}