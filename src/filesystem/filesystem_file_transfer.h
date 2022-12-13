#ifndef FILESYSTEM_FILE_TRANSFER
#define FILESYSTEM_FILE_TRANSFER

#include <iostream>

#include "../socket_message.h"
#include "filesystem_put_request.h"

using namespace std;

class FileSystemFileTransfer {
 public:
  static int64_t send_file(int sockfd, string filename);
  static int64_t recv_file(int sockfd, string filename);
  static void client_file_transfer(int sockfd, string byte_request);
  static void server_file_transfer(int sockfd, string byte_request);

};

#endif