#ifndef FILESYSTEM_GET_VERSION_REQUEST
#define FILESYSTEM_GET_VERSION_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemGetVersionsRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::GET_VERSIONS;
  string sdfsfilename;
  int num_versions;
  string localfilename;
  set<int> versions;
  bool is_initiator;

  FileSystemGetVersionsRequest(){};

  FileSystemGetVersionsRequest(string sdfsfilename, int num_versions, string localfilename, bool is_initiator);
  FileSystemGetVersionsRequest(string sdfsfilename, int num_versions, string localfilename, set<int> versions, bool is_initiator);
  FileSystemGetVersionsRequest(const char* byte) { deserialize(byte); }

  virtual ~FileSystemGetVersionsRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif