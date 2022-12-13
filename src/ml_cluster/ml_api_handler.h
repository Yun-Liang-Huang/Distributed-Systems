#ifndef ML_API_HANDLER
#define ML_API_HANDLER

#include <iostream>
#include <map>

#include "../socket_message.h"

using namespace std;

class MlApiHandler {
 public:
  static SocketMessage* api_handler(const char* buffer);

};

#endif