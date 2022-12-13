#ifndef FILESYSTEM_PUT_REQUEST
#define FILESYSTEM_PUT_REQUEST

#include <iostream>
#include <string>

#include "../socket_message.h"
#include "../socket_message_utils.h"

using namespace std;

class FileSystemPutRequest : public SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type = SOCKET_MESSAGE_TYPE::PUT;
  string localfilename;
  string sdfsfilename;
  int sequence_number;
  bool is_initiator;

  FileSystemPutRequest(){};

  FileSystemPutRequest(string localfilename, string sdfsfilename, int sequence_number, bool is_initiator);
  FileSystemPutRequest(const char* byte) { deserialize(byte); }

  virtual ~FileSystemPutRequest(){};

  const string serialize();

 private:
  void deserialize(const char*);
};

#endif