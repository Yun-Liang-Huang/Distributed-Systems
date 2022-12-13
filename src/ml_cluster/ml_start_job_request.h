#ifndef ML_START_JOB_REQUEST
#define ML_START_JOB_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlStartJobRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::START_JOB;
  string job;

  MlStartJobRequest(){};

  MlStartJobRequest(string job);
  MlStartJobRequest(const char* byte) { deserialize(byte); }

  virtual ~MlStartJobRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif