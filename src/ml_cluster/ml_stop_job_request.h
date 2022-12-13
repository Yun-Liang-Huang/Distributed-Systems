#ifndef ML_STOP_JOB_REQUEST
#define ML_STOP_JOB_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlStopJobRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::STOP_JOB;
  string job;

  MlStopJobRequest(){};

  MlStopJobRequest(string job);
  MlStopJobRequest(const char* byte) { deserialize(byte); }

  virtual ~MlStopJobRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif