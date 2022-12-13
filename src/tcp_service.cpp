
#include "tcp_service.h"

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include <iostream>

#include "filesystem/filesystem_api_handler.h"
#include "filesystem/filesystem_file_transfer.h"
#include "filesystem/filesystem_put_request.h"
#include "membership/member_api_handler.h"
#include "ml_cluster/ml_api_handler.h"
#include "query/query_api_handler.h"
#include "socket_message.h"
#include "socket_message_const.h"
#include "socket_message_utils.h"
#include "utils.h"

using namespace std;

SocketMessage* TcpService::tcp_api_hanlder(int sockfd, const char* buffer) {
  SOCKET_MESSAGE_TYPE message_type =
      SocketMessageUtils::get_message_type(buffer);

  try {
    if (MEMBER_API.count(message_type) != 0) {
      return MemberApiHandler::api_handler(buffer);
    } else if (QUERY_API.count(message_type) != 0) {
      return QueryApiHandler::api_handler(buffer);
    } else if (FILESYSTEM_API.count(message_type) != 0) {
      // server send or receive file
      if (PUT == message_type || GET == message_type ||
          GET_VERSIONS == message_type) {
        FileSystemFileTransfer::server_file_transfer(sockfd, buffer);
      }
      return FileSystemApiHandler::api_handler(buffer);
    } else if (ML_API.count(message_type) != 0) {
      return MlApiHandler::api_handler(buffer);
    } else {
      cerr << "[TCP] No matching message type" << endl;
      throw std::invalid_argument("[TCP] No matching message type");
    }
  } catch (exception e) {
    return new FailedResponse();
  }
}

void TcpService::handle_socket(int new_socket) {
  try {
    char buffer[SOCKET_BUFFER_SIZE] = {0};

    int valread = read(new_socket, buffer, SOCKET_BUFFER_SIZE);

    printf("%s\n", buffer);

    SocketMessage* response = tcp_api_hanlder(new_socket, buffer);
    string response_byte = response->serialize();

    // send response length first
    int response_len = response_byte.size();
    const char* response_len_str = to_string(response_len).c_str();
    cout << Utils::getTime() << "Sending data with len: " << response_len_str
         << endl;
    int n = send(new_socket, response_len_str, strlen(response_len_str), MSG_NOSIGNAL);
    if (n != strlen(response_len_str)) {
      throw runtime_error("Broken pipe");
    }
    read(new_socket, buffer, SOCKET_BUFFER_SIZE);  // OK from client

    int sent_data_len =
        send(new_socket, response_byte.c_str(), response_byte.size(), MSG_NOSIGNAL);

    if (sent_data_len != response_byte.size()) {
      throw runtime_error("Broken pipe");
    }

    // cout << Utils::getTime() << "<<< [" << sent_data_len << "/" <<
    // response_len
    // << "]" << endl;

    delete response;
    close(new_socket);
  } catch (exception& e) {
    cerr << "TCP handle_socket error: " << e.what() << endl;
  }
}

void TcpService::start_tcp_services(int port) {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  if (::bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  cout << Utils::getTime() << "[TCP] Listening on port " << port << endl;

  while (1) {
    if ((new_socket = accept(server_fd, (struct sockaddr*)&address,
                             (socklen_t*)&addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    thread(handle_socket, new_socket).detach();
  }

  // closing the listening socket
  shutdown(server_fd, SHUT_RDWR);
}