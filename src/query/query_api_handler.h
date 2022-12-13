#ifndef QUERY_API_HANDLER
#define QUERY_API_HANDLER

#include <stdbool.h>

#include <iostream>

#include "../socket_message.h"
#include "query_log_response.h"

using namespace std;

class QueryApiHandler {
 public:
  static SocketMessage* api_handler(const char* buffer);
  static QueryLogResponse* queryWorkerLog(QUERY_TYPE query_type, string input,
                                          string file_pattern);
  static QueryLogResponse* queryLog(QUERY_TYPE query_type, string input,
                                    string file_pattern);
};

#endif