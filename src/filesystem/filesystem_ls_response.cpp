#include "filesystem_ls_response.h"

#include <string>
#include <vector>

using namespace std;

FileSystemLsResponse::FileSystemLsResponse(string stored_addresses)
    : stored_addresses(stored_addresses) {}

const string FileSystemLsResponse::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER +
         stored_addresses;
}

void FileSystemLsResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  stored_addresses = tokens[1];
}