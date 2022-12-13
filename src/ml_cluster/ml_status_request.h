#ifndef ML_STATUS_REQUEST
#define ML_STATUS_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlStatusRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::ML_JOB_STATUS;
  string job_id;

  MlStatusRequest(){};

  MlStatusRequest(string job_id);
  MlStatusRequest(const char* byte) { deserialize(byte); }

  virtual ~MlStatusRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif