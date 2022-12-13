#ifndef MEMBER_PING_REQUEST
#define MEMBER_PING_REQUEST

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../filesystem/filesystem_server_context.h"
#include "../ml_cluster/ml_server_context.h"
#include "../socket_message.h"

using namespace std;

class MemberPingRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::MEMBER_PING;
  set<string> ring_topology;
  vector<string> removed_members;
  string failure_detection_message;

  int filesystem_sequence_number;
  unordered_map<string, FileMetadata*> filesystem_file_metadata_map;
  unordered_set<string> filesystem_recovered_failure_servers;

  unordered_map<string, JobMetadata*> ml_job_metadata_map;
  unordered_map<string, JobStatus*> ml_job_status_map;

  MemberPingRequest(){};

  // this will shallow copy the inputted map
  MemberPingRequest(
      set<string> ring_topology, vector<string> removed_members,
      string failure_detection_message, int filesystem_sequence_number,
      unordered_map<string, FileMetadata*> filesystem_file_metadata_map,
      unordered_set<string> filesystem_recovered_failure_servers,
      unordered_map<string, JobMetadata*> ml_job_metadata_map,
      unordered_map<string, JobStatus*> ml_job_status_map);

  MemberPingRequest(const char* byte) { deserialize(byte); }

  virtual ~MemberPingRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif