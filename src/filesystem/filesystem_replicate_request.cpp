#include "filesystem_replicate_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemReplicateRequest::FileSystemReplicateRequest(string sdfsfilename, unordered_set<string> replica_ids)
    : sdfsfilename(sdfsfilename), replica_ids(replica_ids) {}

const string FileSystemReplicateRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfsfilename +
         SOCKET_MESSAGE_DELIMITER + serialize_hashset(replica_ids);
}

void FileSystemReplicateRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfsfilename = tokens[1];
  deserialize_hashset(tokens[2], replica_ids);
}