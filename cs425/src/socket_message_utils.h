#ifndef SOCKET_MESSAGE_UTILS
#define SOCKET_MESSAGE_UTILS

#include <iostream>
#include <vector>

#include "membership/member_get_id_response.h"
#include "membership/member_ping_request.h"
#include "membership/member_request.h"
#include "query/query_log_response.h"
#include "socket_message_const.h"
#include "successful_response.h"
#include "failed_response.h"

using namespace std;

class SocketMessageUtils {
 public:
  static SOCKET_MESSAGE_TYPE get_message_type(const char* byte);

  static vector<string> parse_tokens(const char* byte);

  static SocketMessage* get_socket_response(const char* byte);
};

#endif