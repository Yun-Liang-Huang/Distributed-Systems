#include "filesystem_ls_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemLsRequest::FileSystemLsRequest(string sdfsfilename)
    : sdfsfilename(sdfsfilename) {}

const string FileSystemLsRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfsfilename;
}

void FileSystemLsRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfsfilename = tokens[1];
}