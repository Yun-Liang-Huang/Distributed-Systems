#ifndef FILESYSTEM_GET_REQUEST
#define FILESYSTEM_GET_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemGetRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::GET;
  string sdfsfilename;
  string localfilename;
  int version;
  bool is_initiator;

  FileSystemGetRequest(){};

  FileSystemGetRequest(string sdfsfilename, string localfilename, bool is_initiator);
  FileSystemGetRequest(string sdfsfilename, string localfilename, int version, bool is_initiator);
  FileSystemGetRequest(const char* byte) { deserialize(byte); }

  virtual ~FileSystemGetRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif