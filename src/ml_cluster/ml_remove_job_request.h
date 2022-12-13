#ifndef ML_REMOVE_JOB_REQUEST
#define ML_REMOVE_JOB_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlRemoveJobRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::REMOVE_JOB;
  string job;

  MlRemoveJobRequest(){};

  MlRemoveJobRequest(string job);
  MlRemoveJobRequest(const char* byte) { deserialize(byte); }

  virtual ~MlRemoveJobRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif