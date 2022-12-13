#ifndef ML_RUN_SUBJOB_REQUEST
#define ML_RUN_SUBJOB_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlRunSubjobRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::RUN_SUBJOB;

  string model;
  string testcase;
  string label;
  int offset;
  int limit;
  string output_filename;

  MlRunSubjobRequest(){};

  MlRunSubjobRequest(string model, string testcase, string label, int offset,
                     int limit, string output_filename);
  MlRunSubjobRequest(const char* byte) { deserialize(byte); }

  virtual ~MlRunSubjobRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif