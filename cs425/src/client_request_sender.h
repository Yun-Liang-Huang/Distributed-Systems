#ifndef CLIENT_REQUEST_SENDER
#define CLIENT_REQUEST_SENDER

#include "socket_message.h"

using namespace std;

class RequestSender {
 public:
  static SocketMessage* send_request(const char* address, int port,
                                     SocketMessage* request);
};

#endif