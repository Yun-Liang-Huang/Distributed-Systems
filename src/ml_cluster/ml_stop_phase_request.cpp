#include "ml_stop_phase_request.h"

#include <string>
#include <vector>

using namespace std;

MlStopPhaseRequest::MlStopPhaseRequest(ML_PHASE phase)
    : phase(phase) {}

const string MlStopPhaseRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + to_string(phase);
}

void MlStopPhaseRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  phase = (ML_PHASE)stoi(tokens[1]);
}