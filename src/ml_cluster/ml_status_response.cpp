#include "ml_status_response.h"

#include <string>
#include <vector>

#include "../utils.h"

using namespace std;

const string MlStatusResponse::serialize() {
  vector<string> payload;

  payload.push_back(to_string(message_type));
  payload.push_back(to_string(query_rate));
  payload.push_back(to_string(latest_query_rate));
  payload.push_back(to_string(processed_query_count));
  payload.push_back(to_string(average_query_processing_time));
  payload.push_back(to_string(std_deviation_query_processing_time));
  payload.push_back(to_string(median_query_processing_time));
  payload.push_back(to_string(percentile_90_query_processing_time));
  payload.push_back(to_string(percentile_95_query_processing_time));
  payload.push_back(to_string(percentile_99_query_processing_time));
  payload.push_back(serialize_vector(assigned_workers));

  return Utils::join(payload, SOCKET_MESSAGE_DELIMITER);
}

void MlStatusResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);

  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  query_rate = stof(tokens[1]);
  latest_query_rate = stof(tokens[2]);
  processed_query_count = stoi(tokens[3]);
  average_query_processing_time = stof(tokens[4]);
  std_deviation_query_processing_time = stof(tokens[5]);
  median_query_processing_time = stof(tokens[6]);
  percentile_90_query_processing_time = stof(tokens[7]);
  percentile_95_query_processing_time = stof(tokens[8]);
  percentile_99_query_processing_time = stof(tokens[9]);
  deserialize_vector(tokens[10], assigned_workers);
}