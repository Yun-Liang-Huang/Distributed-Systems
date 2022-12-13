#include "ml_status_request.h"

#include <string>
#include <vector>

using namespace std;

MlStatusRequest::MlStatusRequest(string job_id) : job_id(job_id) {}

const string MlStatusRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + job_id;
}

void MlStatusRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  job_id = tokens[1];
}