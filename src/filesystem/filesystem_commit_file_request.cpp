#include "filesystem_commit_file_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemCommitFileRequest::FileSystemCommitFileRequest(string sdfs_filename,
                                                         int sequence_number)
    : sdfs_filename(sdfs_filename), sequence_number(sequence_number) {}

const string FileSystemCommitFileRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfs_filename +
         SOCKET_MESSAGE_DELIMITER + to_string(sequence_number);
}

void FileSystemCommitFileRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfs_filename = tokens[1];
  sequence_number = stoi(tokens[2]);
}