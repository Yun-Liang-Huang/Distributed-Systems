#include "filesystem_get_versions_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemGetVersionsRequest::FileSystemGetVersionsRequest(string sdfsfilename, int num_versions, 
                                  string localfilename, set<int> versions, bool is_initiator)
    : sdfsfilename(sdfsfilename), num_versions(num_versions), localfilename(localfilename), versions(versions), is_initiator(is_initiator) {}

FileSystemGetVersionsRequest::FileSystemGetVersionsRequest(string sdfsfilename, int num_versions, 
                                  string localfilename, bool is_initiator)
    : sdfsfilename(sdfsfilename), num_versions(num_versions), localfilename(localfilename), versions(), is_initiator(is_initiator) {}

const string FileSystemGetVersionsRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + sdfsfilename +
         SOCKET_MESSAGE_DELIMITER + to_string(num_versions) +
         SOCKET_MESSAGE_DELIMITER + localfilename +
         SOCKET_MESSAGE_DELIMITER + serialize_set(versions) +
         SOCKET_MESSAGE_DELIMITER + to_string(is_initiator);
}

void FileSystemGetVersionsRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  sdfsfilename = tokens[1];
  num_versions = stoi(tokens[2]);
  localfilename = tokens[3];
  deserialize_set(tokens[4], versions);
  is_initiator = stoi(tokens[5]);
}