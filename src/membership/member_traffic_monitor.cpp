#include "member_traffic_monitor.h"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace MEMBER_TRAFFIC_MONITOR {

uint64_t udp_output_bits = 0;
const int MONITOR_INTERVAL_MS = 20000;  // every 20s

void printAverageTraffic() {
  uint64_t average_bps = udp_output_bits / (MONITOR_INTERVAL_MS / 1000);
  std::cout << "Average Traffic: " << average_bps << " BPS" << std::endl;

  udp_output_bits = 0;
}

}  // namespace MEMBER_TRAFFIC_MONITOR