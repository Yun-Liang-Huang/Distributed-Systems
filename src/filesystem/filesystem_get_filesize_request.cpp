#include "filesystem_get_filesize_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemGetFilesizeRequest::FileSystemGetFilesizeRequest(string sdfsfilename, int version)
    : sdfsfilename(sdfsfilename), version(version) {}

const string FileSystemGetFilesizeRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfsfilename +
         SOCKET_MESSAGE_DELIMITER + to_string(version);
}

void FileSystemGetFilesizeRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfsfilename = tokens[1];
  version = stoi(tokens[2]);
}