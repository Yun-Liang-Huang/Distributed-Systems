#ifndef ML_STATUS_RESPONSE
#define ML_STATUS_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class MlStatusResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::ML_JOB_STATUS;
  double query_rate = 0;  // query per sec since job start
  double latest_query_rate = 0;  // query per sec of the last 10 sec

  int processed_query_count =
      0;  // the number of queries processed so far (running
          // count, since the start of the model).

  // second per query statistics:
  double average_query_processing_time = 0;
  double std_deviation_query_processing_time = 0;
  double median_query_processing_time = 0;
  double percentile_90_query_processing_time = 0;
  double percentile_95_query_processing_time = 0;
  double percentile_99_query_processing_time = 0;

  vector<string> assigned_workers;

  MlStatusResponse(){};

  MlStatusResponse(const char* byte) { deserialize(byte); }

  virtual ~MlStatusResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif