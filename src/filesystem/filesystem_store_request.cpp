#include "filesystem_store_request.h"

#include <string>
#include <vector>

using namespace std;

const string FileSystemStoreRequest::serialize() {
  return to_string(message_type);
}

void FileSystemStoreRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
}