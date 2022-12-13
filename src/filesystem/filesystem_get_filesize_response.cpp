#include "filesystem_get_filesize_response.h"

#include <string>
#include <vector>

using namespace std;

FileSystemGetFilesizeResponse::FileSystemGetFilesizeResponse(int64_t filesize)
    : filesize(filesize) {}

const string FileSystemGetFilesizeResponse::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + to_string(filesize);
}

void FileSystemGetFilesizeResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  filesize = stoi(tokens[1]);
}