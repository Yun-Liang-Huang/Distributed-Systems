#ifndef ML_STOP_PHASE_REQUEST
#define ML_STOP_PHASE_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlStopPhaseRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::STOP_PHASE;
  ML_PHASE phase;

  MlStopPhaseRequest(){};

  MlStopPhaseRequest(ML_PHASE phase);
  MlStopPhaseRequest(const char* byte) { deserialize(byte); }

  virtual ~MlStopPhaseRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif