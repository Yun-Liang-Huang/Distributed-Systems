#include "filesystem_delete_file_metadata_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemDeleteFileMetadataRequest::FileSystemDeleteFileMetadataRequest(string sdfs_filename)
    : sdfs_filename(sdfs_filename) {}

const string FileSystemDeleteFileMetadataRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfs_filename;
}

void FileSystemDeleteFileMetadataRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfs_filename = tokens[1];
}