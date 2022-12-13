#include "ml_run_subjob_response.h"

#include "../utils.h"

using namespace std;

MlRunSubjobResponse::MlRunSubjobResponse(string output_filename,
                                         int done_query_count,
                                         int accurate_query_count)
    : output_filename(output_filename),
      done_query_count(done_query_count),
      accurate_query_count(accurate_query_count) {}

const string MlRunSubjobResponse::serialize() {
  vector<string> payload = {to_string(message_type), output_filename,
                            to_string(done_query_count),
                            to_string(accurate_query_count)};

  return Utils::join(payload, SOCKET_MESSAGE_DELIMITER);
}

void MlRunSubjobResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  output_filename = tokens[1];
  done_query_count = stoi(tokens[2]);
  accurate_query_count = stoi(tokens[3]);
}