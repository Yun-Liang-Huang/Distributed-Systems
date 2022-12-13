#include "client_request_sender.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "filesystem/filesystem_file_transfer.h"
#include "filesystem/filesystem_put_request.h"
#include "socket_message.h"
#include "socket_message_const.h"
#include "socket_message_utils.h"
#include "utils.h"

#define INVALID_SOCKET -1

using namespace std;

// credit: https://stackoverflow.com/a/52728208
int gen_socket(const char* hostname, int port) {
  int base_socket, err;
  struct addrinfo hints = {}, *addrs;
  char port_str[16] = {};

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  sprintf(port_str, "%d", port);

  err = getaddrinfo(hostname, port_str, &hints, &addrs);
  if (err != 0) {
    fprintf(stderr, "%s: %s\n", hostname, gai_strerror(err));
    return -1;
  }

  for (struct addrinfo* addr = addrs; addr != NULL; addr = addr->ai_next) {
    base_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (base_socket == -1) {
      err = errno;
      break;
    }

    if (connect(base_socket, addr->ai_addr, addr->ai_addrlen) == 0) break;

    err = errno;

    close(base_socket);
    base_socket = INVALID_SOCKET;
  }

  freeaddrinfo(addrs);

  if (base_socket == INVALID_SOCKET) {
    ostringstream oss(ostringstream::ate);
    oss << hostname << ": " << strerror(err) << endl;
  }

  return base_socket;
}

SocketMessage* RequestSender::send_request(const char* hostname, int port,
                                           SocketMessage* request) {
  char buffer[SOCKET_BUFFER_SIZE] = {0};
  ostringstream oss(ostringstream::ate);
  int client_fd = gen_socket(hostname, port);

  if (client_fd < 0) {
    cerr << "Connection Failed" << endl;
    return new FailedResponse();
  }

  const string byte_request = request->serialize();
  int n =
      send(client_fd, byte_request.c_str(), byte_request.size(), MSG_NOSIGNAL);
  if (n != byte_request.size()) {
    return new FailedResponse();
  }
  // cout << "<<< [" << hostname << ":" << port << "] " << request->message_type
  //      << endl;

  // client send or receive file
  SOCKET_MESSAGE_TYPE message_type =
      SocketMessageUtils::get_message_type(byte_request.c_str());
  try {
    if (PUT == message_type || GET == message_type ||
        GET_VERSIONS == message_type) {
      FileSystemFileTransfer::client_file_transfer(client_fd, byte_request);
    }
  } catch (exception& e) {
    cerr << "connection failed: " << e.what() << endl;
    return new FailedResponse();
  }

  // fetch the response length
  recv(client_fd, buffer, SOCKET_BUFFER_SIZE, 0);

  stringstream strstream(buffer);
  int response_len;
  strstream >> response_len;
  int read_data_len = 0;

  n = send(client_fd, "OK", 2,
           MSG_NOSIGNAL);  // tell the server that client is ready to receive
                           // data packet
  if (n != 2) {
    return new FailedResponse();
  }

  // read splitted packets one by one
  while (read_data_len < response_len) {
    char buffer[SOCKET_BUFFER_SIZE] = {0};
    int receive_byte = recv(client_fd, buffer, SOCKET_BUFFER_SIZE, 0);
    read_data_len += receive_byte;
    // cout << ">>> [" << hostname << ":" << port << "] [" << read_data_len <<
    // "/" << response_len << "]" << endl;

    // never treat something that is not string as a string, because you don't
    // know the len of the string.
    string temp = buffer;
    oss << temp.substr(0, receive_byte);
  }

  // cout << ">>> [" << hostname << ":" << port << "] [" << read_data_len << "/"
  //      << response_len << "]" << endl;

  string response_str = oss.str();

  SocketMessage* response =
      SocketMessageUtils::get_socket_response(response_str.c_str());

  // closing the connected socket
  close(client_fd);
  return response;
}
