#ifndef QUERY_LOG_RESPONSE
#define QUERY_LOG_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"

using namespace std;

class QueryLogResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::QUERY_LOG;
  string logs;
  int match_line_count;
  int total_line_count;

  QueryLogResponse(){};

  QueryLogResponse(string, int, int);
  QueryLogResponse(const char* byte);

  virtual ~QueryLogResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif