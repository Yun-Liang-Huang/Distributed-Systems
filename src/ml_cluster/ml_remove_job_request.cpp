#include "ml_remove_job_request.h"

#include <string>
#include <vector>

using namespace std;

MlRemoveJobRequest::MlRemoveJobRequest(string job)
    : job(job) {}

const string MlRemoveJobRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + job;
}

void MlRemoveJobRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  job = tokens[1];
}