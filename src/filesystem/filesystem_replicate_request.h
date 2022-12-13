#ifndef FILESYSTEM_REPLICATE_REQUEST
#define FILESYSTEM_REPLICATE_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemReplicateRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::REPLICATE;
  string sdfsfilename;
  unordered_set<string> replica_ids;

  FileSystemReplicateRequest(){};

  FileSystemReplicateRequest(string sdfsfilename, unordered_set<string> replica_ids);
  FileSystemReplicateRequest(const char* byte) { deserialize(byte); }

  virtual ~FileSystemReplicateRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif