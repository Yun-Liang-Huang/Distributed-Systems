#include "ml_stop_job_request.h"

#include <string>
#include <vector>

using namespace std;

MlStopJobRequest::MlStopJobRequest(string job)
    : job(job) {}

const string MlStopJobRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + job;
}

void MlStopJobRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  job = tokens[1];
}