#include "ml_run_subjob_request.h"

#include "../utils.h"

using namespace std;

MlRunSubjobRequest::MlRunSubjobRequest(string model, string testcase,
                                       string label, int offset, int limit,
                                       string output_filename)
    : model(model),
      testcase(testcase),
      label(label),
      offset(offset),
      limit(limit),
      output_filename(output_filename) {}

const string MlRunSubjobRequest::serialize() {
  vector<string> payload = {to_string(message_type),
                            model,
                            testcase,
                            label,
                            to_string(offset),
                            to_string(limit),
                            output_filename};

  return Utils::join(payload, SOCKET_MESSAGE_DELIMITER);
}

void MlRunSubjobRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  model = tokens[1];
  testcase = tokens[2];
  label = tokens[3];
  offset = stoi(tokens[4]);
  limit = stoi(tokens[5]);
  output_filename = tokens[6];
}