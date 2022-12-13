#include "filesystem_put_request.h"

#include <string>
#include <vector>

using namespace std;

FileSystemPutRequest::FileSystemPutRequest(string localfilename, string sdfsfilename, int sequence_number, bool is_initiator)
    : localfilename(localfilename), sdfsfilename(sdfsfilename), sequence_number(sequence_number), is_initiator(is_initiator) {}

const string FileSystemPutRequest::serialize() {
  return to_string(message_type) + SOCKET_MESSAGE_DELIMITER + localfilename +
         SOCKET_MESSAGE_DELIMITER + sdfsfilename +
         SOCKET_MESSAGE_DELIMITER + to_string(sequence_number) +
         SOCKET_MESSAGE_DELIMITER + to_string(is_initiator);
}

void FileSystemPutRequest::deserialize(const char* byte) {
  vector<string> tokens = SocketMessageUtils::parse_tokens(byte);
  message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);
  localfilename = tokens[1];
  sdfsfilename = tokens[2];
  sequence_number = stoi(tokens[3]);
  is_initiator = stoi(tokens[4]);
}