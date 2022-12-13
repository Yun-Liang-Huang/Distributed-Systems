#include "filesystem_get_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemGetRequest::FileSystemGetRequest(string sdfsfilename, string localfilename, int version, bool is_initiator)
    : sdfsfilename(sdfsfilename), localfilename(localfilename), version(version), is_initiator(is_initiator) {}

FileSystemGetRequest::FileSystemGetRequest(string sdfsfilename, string localfilename, bool is_initiator)
    : sdfsfilename(sdfsfilename), localfilename(localfilename), version(0), is_initiator(is_initiator) {}

const string FileSystemGetRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfsfilename +
         SOCKET_MESSAGE_DELIMITER + localfilename +
         SOCKET_MESSAGE_DELIMITER + to_string(version) +
         SOCKET_MESSAGE_DELIMITER + to_string(is_initiator);
}

void FileSystemGetRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfsfilename = tokens[1];
  localfilename = tokens[2];
  version = stoi(tokens[3]);
  is_initiator = stoi(tokens[4]);
}