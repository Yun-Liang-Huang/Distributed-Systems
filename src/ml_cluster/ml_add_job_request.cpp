#include "ml_add_job_request.h"

#include <string>
#include <vector>

using namespace std;

MlAddJobRequest::MlAddJobRequest(string job, string model, string testset,
                                 string label, int batch_size)
    : job(job),
      model(model),
      testset(testset),
      label(label),
      batch_size(batch_size) {}

const string MlAddJobRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + job +
         SOCKET_MESSAGE_DELIMITER + model + SOCKET_MESSAGE_DELIMITER + testset +
         SOCKET_MESSAGE_DELIMITER + label + SOCKET_MESSAGE_DELIMITER +
         to_string(batch_size);
}

void MlAddJobRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  job = tokens[1];
  model = tokens[2];
  testset = tokens[3];
  label = tokens[4];
  batch_size = stoi(tokens[5]);
}