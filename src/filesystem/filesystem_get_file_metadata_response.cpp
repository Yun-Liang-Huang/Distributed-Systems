#include "filesystem_get_file_metadata_response.h"

#include <string>
#include <vector>

using namespace std;

FileSystemGetFileMetadataResponse::FileSystemGetFileMetadataResponse(
    unordered_set<string> replica_ids, int sequence_number)
    : replica_ids(replica_ids), sequence_number(sequence_number) {}

const string FileSystemGetFileMetadataResponse::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER +
         serialize_hashset(replica_ids) + SOCKET_MESSAGE_DELIMITER +
         to_string(sequence_number);
}

void FileSystemGetFileMetadataResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  deserialize_hashset(tokens[1], replica_ids);
  sequence_number = stoi(tokens[2]);
}