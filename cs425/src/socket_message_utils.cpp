
#include "socket_message_utils.h"

#include "filesystem/filesystem_get_file_metadata_response.h"
#include "filesystem/filesystem_get_filesize_response.h"
#include "filesystem/filesystem_ls_response.h"
#include "filesystem/filesystem_replicate_response.h"
#include "filesystem/filesystem_store_response.h"
#include "ml_cluster/ml_run_subjob_response.h"
#include "ml_cluster/ml_status_response.h"

using namespace std;

SOCKET_MESSAGE_TYPE SocketMessageUtils::get_message_type(const char* byte) {
  try {
    string str_byte = string(byte);
    int end = str_byte.find(SOCKET_MESSAGE_DELIMITER);
    string token = str_byte.substr(0, end);
    return (SOCKET_MESSAGE_TYPE)stoi(token);
  } catch (exception e) {
    cerr << "Failed to parse message type" << endl;
  }
}

vector<string> SocketMessageUtils::parse_tokens(const char* byte) {
  string str_byte = string(byte);
  size_t next = 0, prev = 0;
  string token;
  vector<string> tokens;

  while ((next = str_byte.find(SOCKET_MESSAGE_DELIMITER, prev)) !=
         string::npos) {
    token = str_byte.substr(prev, next - prev);
    tokens.push_back(token);
    prev = next + SOCKET_MESSAGE_DELIMITER.size();
  }

  token = str_byte.substr(prev);
  tokens.push_back(token);

  return tokens;
}

SocketMessage* SocketMessageUtils::get_socket_response(const char* byte) {
  if (*byte == 0) {
    return new FailedResponse();
  }

  vector<string> tokens = parse_tokens(byte);
  try {
    SOCKET_MESSAGE_TYPE message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);

    SocketMessage* socket_message;
    switch (message_type) {
      case SOCKET_MESSAGE_TYPE::QUERY_LOG:
      case SOCKET_MESSAGE_TYPE::QUERY_WORKER_LOG:
        socket_message = new QueryLogResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::SUCCESSFUL:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::FAILED:
        socket_message = new FailedResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::MEMBER_GET_ID:
        socket_message = new MemberGetIdResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::PUT:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::GET:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::DELETE:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::LS:
        socket_message = new FileSystemLsResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::STORE:
        socket_message = new FileSystemStoreResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::GET_VERSIONS:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::GET_FILE_METADATA:
        socket_message = new FileSystemGetFileMetadataResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::COMMIT_FILE:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::DELETE_FILE_METADATA:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::REPLICATE:
        socket_message = new FileSystemReplicateResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::GET_FILESIZE:
        socket_message = new FileSystemGetFilesizeResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::ADD_JOB:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::REMOVE_JOB:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::START_JOB:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::STOP_JOB:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::START_PHASE:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::STOP_PHASE:
        socket_message = new SuccessfulResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::RUN_SUBJOB:
        socket_message = new MlRunSubjobResponse(byte);
        break;
      case SOCKET_MESSAGE_TYPE::ML_JOB_STATUS:
        socket_message = new MlStatusResponse(byte);
        break;
      default:
        cout << "[socket message utils] message not found" << endl;
        socket_message = nullptr;
    }

    if (socket_message) {
      socket_message->message_type = message_type;
    }

    return socket_message;
  } catch (invalid_argument& e) {
    cout << "Failed to parse socket response: " << e.what() << endl;
    return nullptr;
  }
}