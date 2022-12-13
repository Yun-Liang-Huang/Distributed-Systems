#include "filesystem_replicate_response.h"

#include <string>
#include <vector>

using namespace std;

FileSystemReplicateResponse::FileSystemReplicateResponse(string sdfsfilename, unordered_set<int> replicated_versions)
    : sdfsfilename(sdfsfilename), replicated_versions(replicated_versions) {}

const string FileSystemReplicateResponse::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfsfilename +
         SOCKET_MESSAGE_DELIMITER + serialize_hashset(replicated_versions);
}

void FileSystemReplicateResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfsfilename = tokens[1];
  deserialize_hashset(tokens[2], replicated_versions);
}