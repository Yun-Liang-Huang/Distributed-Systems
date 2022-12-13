#ifndef ML_START_PHASE_REQUEST
#define ML_START_PHASE_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlStartPhaseRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::START_PHASE;
  ML_PHASE phase;

  MlStartPhaseRequest(){};

  MlStartPhaseRequest(ML_PHASE phase);
  MlStartPhaseRequest(const char* byte) { deserialize(byte); }

  virtual ~MlStartPhaseRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif