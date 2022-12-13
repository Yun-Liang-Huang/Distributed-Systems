#include "filesystem_get_file_metadata_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemGetFileMetadataRequest::FileSystemGetFileMetadataRequest(
    string sdfs_filename)
    : sdfs_filename(sdfs_filename) {}

FileSystemGetFileMetadataRequest::FileSystemGetFileMetadataRequest(
    string sdfs_filename, unordered_set<string> base_replica_ids)
    : sdfs_filename(sdfs_filename), base_replica_ids(base_replica_ids) {}

const string FileSystemGetFileMetadataRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfs_filename +
         SOCKET_MESSAGE_DELIMITER + serialize_hashset(base_replica_ids);
}

void FileSystemGetFileMetadataRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfs_filename = tokens[1];
  deserialize_hashset(tokens[2], base_replica_ids);
}