#ifndef FILESYSTEM_API_HANDLER
#define FILESYSTEM_API_HANDLER

#include <iostream>
#include <map>

#include "../socket_message.h"

using namespace std;

class FileSystemApiHandler {
 public:
  static SocketMessage* api_handler(const char* buffer);
  static void get_replica(string replica_id, string sdfsfilename, int version);
  static void get_versions_replica(string replica_id, string sdfsfilename, int num_versions, set<int> versions);

};

#endif