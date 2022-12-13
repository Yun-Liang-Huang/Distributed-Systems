#ifndef TCP_SERVICE
#define TCP_SERVICE

#include <iostream>

#include "socket_message.h"
#include "socket_message_const.h"

using namespace std;

class TcpService {
 public:
  static void start_tcp_services(int port);
  static void handle_socket(int new_socket);
  static SocketMessage* tcp_api_hanlder(int sockfd, const char* buffer);
};

#endif