#include "filesystem_store_response.h"

#include <string>
#include <vector>

using namespace std;

FileSystemStoreResponse::FileSystemStoreResponse(string stored_files)
    : stored_files(stored_files) {}

const string FileSystemStoreResponse::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER +
         stored_files;
}

void FileSystemStoreResponse::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  stored_files = tokens[1];
}