#ifndef MEMBER_API_HANDLER
#define MEMBER_API_HANDLER

#include <stdbool.h>

#include <iostream>
#include <unordered_map>

#include "../socket_message.h"
#include "member_ping_request.h"

using namespace std;

class MemberApiHandler {
 public:
  static SocketMessage* api_handler(const char* buffer);
  static void udp_ping_handler(MemberPingRequest* request);
  static void udp_receiver(int port);
  static void start_udp_receiver(int port);
  static string udp_sender(string ip_address, int port, SocketMessage* request);
  static void ping_k_member();
};

#endif