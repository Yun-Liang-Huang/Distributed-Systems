#ifndef _MEMBER_TRAFFIC_MONITOR
#define _MEMBER_TRAFFIC_MONITOR

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

extern uint64_t udp_output_bits;
extern const int MONITOR_INTERVAL_MS;

void printAverageTraffic();

}  // namespace MEMBER_TRAFFIC_MONITOR

#endif