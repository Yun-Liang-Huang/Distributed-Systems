#ifndef ML_ADD_JOB_REQUEST
#define ML_ADD_JOB_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlAddJobRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::ADD_JOB;
  string job;
  string model;
  string testset;
  string label;
  int batch_size;

  MlAddJobRequest(){};

  MlAddJobRequest(string job, string model, string testset, string label,
                  int batch_size);
  MlAddJobRequest(const char* byte) { deserialize(byte); }

  virtual ~MlAddJobRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif