#ifndef QUERY_REQUEST
#define QUERY_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"

using namespace std;

class QueryRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type;
  QUERY_TYPE query_type;
  string input;
  string file_pattern;

  QueryRequest(){};

  QueryRequest(SOCKET_MESSAGE_TYPE message_type, QUERY_TYPE query_type,
               string input, string file_pattern);
  QueryRequest(const char* byte) { deserialize(byte); }

  virtual ~QueryRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif