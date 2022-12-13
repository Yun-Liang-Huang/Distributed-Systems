#include "member_ping_request.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "../socket_message.h"
#include "../socket_message_const.h"
#include "../socket_message_utils.h"

using namespace std;

MemberPingRequest::MemberPingRequest(
    set<string> ring_topology, vector<string> removed_members,
    string failure_detection_message, int filesystem_sequence_number,
    unordered_map<string, FileMetadata*> filesystem_file_metadata_map,
    unordered_set<string> filesystem_recovered_failure_servers,
    unordered_map<string, JobMetadata*> ml_job_metadata_map,
    unordered_map<string, JobStatus*> ml_job_status_map)
    : ring_topology(ring_topology),
      removed_members(removed_members),
      failure_detection_message(failure_detection_message),
      filesystem_sequence_number(filesystem_sequence_number),
      filesystem_file_metadata_map(filesystem_file_metadata_map),
      filesystem_recovered_failure_servers(
          filesystem_recovered_failure_servers),
      ml_job_metadata_map(ml_job_metadata_map),
      ml_job_status_map(ml_job_status_map) {}

const string MemberPingRequest::serialize() {
  // packing filesystem_file_metadata_map
  unordered_map<string, string> file_metadata_map;
  for (auto const& [key, value] : filesystem_file_metadata_map) {
    file_metadata_map[key] = value->serialize();
  }

  // packing ml_job_metadata_map
  unordered_map<string, string> job_metadata_map;
  for (auto const& [key, value] : ml_job_metadata_map) {
    job_metadata_map[key] = value->serialize();
  }

  // packing ml_job_status_map
  unordered_map<string, string> job_status_map;
  for (auto const& [key, value] : ml_job_status_map) {
    job_status_map[key] = value->serialize();
  }

  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER +
         serialize_set(ring_topology) + SOCKET_MESSAGE_DELIMITER +
         serialize_vector(removed_members) + SOCKET_MESSAGE_DELIMITER +
         failure_detection_message + SOCKET_MESSAGE_DELIMITER +
         to_string(filesystem_sequence_number) + SOCKET_MESSAGE_DELIMITER +
         serialize_hashmap(file_metadata_map) + SOCKET_MESSAGE_DELIMITER +
         serialize_hashset(filesystem_recovered_failure_servers) +
         SOCKET_MESSAGE_DELIMITER + serialize_hashmap(job_metadata_map) +
         SOCKET_MESSAGE_DELIMITER + serialize_hashmap(job_status_map);
}

void MemberPingRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  deserialize_set(tokens[1], ring_topology);
  deserialize_vector(tokens[2], removed_members);
  failure_detection_message = tokens[3];
  filesystem_sequence_number = stoi(tokens[4]);

  // unpacking filesystem_file_metadata_map
  unordered_map<string, string> file_metadata_map;
  deserialize_hashmap(tokens[5], file_metadata_map);
  for (auto const& [key, value] : file_metadata_map) {
    filesystem_file_metadata_map[key] = FileMetadata::deserialize(value);
  }

  deserialize_hashset(tokens[6], filesystem_recovered_failure_servers);

  // unpacking ml_job_metadata_map
  unordered_map<string, string> job_metadata_map;
  deserialize_hashmap(tokens[7], job_metadata_map);
  for (auto const& [key, value] : job_metadata_map) {
    ml_job_metadata_map[key] = JobMetadata::deserialize(value);
  }

  // unpacking ml_job_status_map
  unordered_map<string, string> job_status_map;
  deserialize_hashmap(tokens[8], job_status_map);
  for (auto const& [key, value] : job_status_map) {
    ml_job_status_map[key] = JobStatus::deserialize(value);
  }
}
