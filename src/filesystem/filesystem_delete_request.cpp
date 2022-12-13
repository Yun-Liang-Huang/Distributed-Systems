#include "filesystem_delete_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemDeleteRequest::FileSystemDeleteRequest(string sdfsfilename, bool is_initiator)
    : sdfsfilename(sdfsfilename), is_initiator(is_initiator) {}

const string FileSystemDeleteRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfsfilename +
         SOCKET_MESSAGE_DELIMITER + to_string(is_initiator);
}

void FileSystemDeleteRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfsfilename = tokens[1];
  is_initiator = stoi(tokens[2]);
}