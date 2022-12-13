#ifndef FILESYSTEM_REPLICATE_RESPONSE
#define FILESYSTEM_REPLICATE_RESPONSE

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemReplicateResponse : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::REPLICATE;
  string sdfsfilename;
  unordered_set<int> replicated_versions;

  FileSystemReplicateResponse(){};

  FileSystemReplicateResponse(string sdfsfilename, unordered_set<int> replicated_versions);
  FileSystemReplicateResponse(const char* byte) { deserialize(byte); }

  virtual ~FileSystemReplicateResponse(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif