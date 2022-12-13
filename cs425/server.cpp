#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* srand, rand */
#include <string.h>
#include <sys/socket.h>
#include <time.h> /* time */

#include <iostream>
#include <sstream>
#include <thread>

#include "src/filesystem/filesystem_server_context.h"
#include "src/filesystem/filesystem_utils.h"
#include "src/membership/member_server_context.h"
#include "src/membership/member_traffic_monitor.h"
#include "src/membership/member_utils.h"
#include "src/ml_cluster/ml_server_context.h"
#include "src/ml_cluster/ml_utils.h"
#include "src/socket_message_utils.h"
#include "src/tcp_service.h"
#include "src/utils.h"

using namespace std;

#define PORT 8088

int main(int argc, char const* argv[]) {
  // freopen("MP2.txt", "w", stdout);

  srand(time(NULL));
  int port = (argc >= 2) ? atoi(argv[1]) : PORT;
  MemberServerContext::CreateContext(port);
  FileSystemServerContext::CreateContext();
  MlServerContext::CreateContext();

  thread tcp_service_thread(TcpService::start_tcp_services, port);
  thread udp_receiver_thread(MemberApiHandler::start_udp_receiver, port);
  thread udp_sender_thread(MemberApiHandler::ping_k_member);
  Utils::start(5000, FileSystemUtils::handle_server_failure);
  Utils::start(5000, MemberUtils::printMembershipList);
  Utils::start(MEMBER_TRAFFIC_MONITOR::MONITOR_INTERVAL_MS,
               MEMBER_TRAFFIC_MONITOR::printAverageTraffic);

  Utils::start(5000, MlUtils::resource_management_cycle);

  tcp_service_thread.join();
  udp_receiver_thread.join();
  udp_sender_thread.join();

  MemberServerContext::ReleaseContext();
  return 0;
}
