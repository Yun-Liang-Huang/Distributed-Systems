#ifndef ML_RUN_SUBJOB_RESPONSE
#define ML_RUN_SUBJOB_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlRunSubjobResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::RUN_SUBJOB;

  string output_filename;
  int done_query_count;
  int accurate_query_count;

  MlRunSubjobResponse(){};

  MlRunSubjobResponse(string output_filename, int done_query_count,
                      int accurate_query_count);
  MlRunSubjobResponse(const char* byte) { deserialize(byte); }

  virtual ~MlRunSubjobResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif