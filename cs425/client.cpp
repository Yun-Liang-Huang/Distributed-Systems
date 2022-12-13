#include <chrono>
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>

#include "src/client_request_sender.h"
#include "src/filesystem/filesystem_delete_request.h"
#include "src/filesystem/filesystem_get_request.h"
#include "src/filesystem/filesystem_get_versions_request.h"
#include "src/filesystem/filesystem_ls_request.h"
#include "src/filesystem/filesystem_put_request.h"
#include "src/filesystem/filesystem_store_request.h"
#include "src/filesystem/filesystem_store_response.h"
#include "src/membership/member_request.h"
#include "src/ml_cluster/ml_add_job_request.h"
#include "src/ml_cluster/ml_remove_job_request.h"
#include "src/ml_cluster/ml_start_job_request.h"
#include "src/ml_cluster/ml_start_phase_request.h"
#include "src/ml_cluster/ml_status_request.h"
#include "src/ml_cluster/ml_status_response.h"
#include "src/ml_cluster/ml_stop_job_request.h"
#include "src/ml_cluster/ml_stop_phase_request.h"
#include "src/query/query_log_response.h"
#include "src/query/query_request.h"

using namespace std;

string receiver_hostname = "localhost";
string receiver_port = "8088";

SocketMessage* parseQueryArgs(vector<string>);
SocketMessage* parseMembershipArgs(vector<string>);
SocketMessage* parseSdfsArgs(vector<string>);
SocketMessage* parseMlArgs(vector<string>);
SocketMessage* checkArgs(int argc, char const* argv[]);
void print_ml_job_status(MlStatusResponse* job_status);

int main(int argc, char const* argv[]) {
  // Set up timer
  auto start = chrono::system_clock::now();
  time_t start_time = chrono::system_clock::to_time_t(start);

  SocketMessage* request = checkArgs(argc, argv);
  SocketMessage* response = RequestSender::send_request(
      receiver_hostname.c_str(), stoi(receiver_port), request);

  vector<string> tokens =
      SocketMessageUtils::parse_tokens(response->serialize().c_str());
  SOCKET_MESSAGE_TYPE message_type = (SOCKET_MESSAGE_TYPE)stoi(tokens[0]);

  if (LS == message_type) {
    cout << "stored addresses: " << endl;
    cout << tokens[1] << endl;
  } else if (STORE == message_type) {
    cout << "stored files: " << endl;
    cout << tokens[1] << endl;
  } else if (SUCCESSFUL == message_type) {
    cout << "Request success" << endl;
  } else if (FAILED == message_type) {
    cout << "Request failed" << endl;
  } else if (ML_JOB_STATUS == message_type) {
    print_ml_job_status((MlStatusResponse*)response);
  }

  if (response) {
    delete response;
  }

  delete request;

  auto end = chrono::system_clock::now();

  chrono::duration<double> elapsed_seconds = end - start;
  time_t end_time = chrono::system_clock::to_time_t(end);

  cout
      // << "Start execution at " << ctime(&start_time)
      // << "Finished execution at " << ctime(&end_time)
      << "Elapsed time: " << elapsed_seconds.count() << "s" << endl;

  return 0;
}

SocketMessage* checkArgs(int argc, char const* argv[]) {
  if (argc > 1) {
    vector<string> all_args;
    all_args.assign(argv + 1, argv + argc);
    if (all_args[0] == "query") {
      return parseQueryArgs(all_args);
    } else if (all_args[0] == "membership") {
      return parseMembershipArgs(all_args);
    } else if (all_args[0] == "sdfs") {
      return parseSdfsArgs(all_args);
    } else if (all_args[0] == "ml") {
      return parseMlArgs(all_args);
    }
  }

  cout << "./client [query|membership|sdfs]" << endl;
  // exit(1);
}

SocketMessage* parseQueryArgs(vector<string> args) {
  if (args.size() <= 2 || args.size() >= 5) {
    cout << "./client query [-i|-regex] <query pattern> <filename>" << endl;
    exit(1);
  }

  QUERY_TYPE query_type;
  string query_pattern;
  string filename;

  if (args.size() == 3) {
    query_type = STRING_SEARCH;
    query_pattern = args[1];
    filename = args[2];
  } else if (args.size() == 4) {
    if (args[1] == "-i") {
      query_type = CASE_INSENSITIVE;
    } else if (args[1] == "-regex") {
      query_type = REGULAR_EXPRESSION;
    } else {
      cout << "Query type not found" << endl;
      cout << "./client query [-i|-regex] <query pattern> <filename>" << endl;
      exit(1);
    }
    query_pattern = args[2];
    filename = args[3];
  }

  return new QueryRequest(QUERY_LOG, query_type, query_pattern, filename);
}

SocketMessage* parseMembershipArgs(vector<string> args) {
  if (args.size() <= 2) {
    cout << "Invalid parameters count" << endl;
    cout << "./client membership [join|leave|list]" << endl;
    exit(1);
  }

  SocketMessage* request;
  SOCKET_MESSAGE_TYPE message_type;

  if (args[1] == "join") {
    message_type = JOIN;
    string new_member_host;
    string new_member_port;
    receiver_hostname = "localhost";
    receiver_port = "8088";

    if (args.size() == 4) {
      new_member_host = args[2];
      new_member_port = args[3];
    } else if (args.size() == 6) {
      new_member_host = args[2];
      new_member_port = args[3];
      receiver_hostname = args[4];
      receiver_port = args[5];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client membership join <new member host> <new member port> "
              "[<introducer "
              "host> <introducer port>]"
           << endl;
      exit(1);
    }

    request = new MemberRequest(message_type, new_member_host, new_member_port);

  } else if (args[1] == "leave") {
    if (args.size() != 4) {
      cout << "Invalid parameters count" << endl;
      cout << "./client membership leave <member host> <member port>" << endl;
      exit(1);
    }

    message_type = LEAVE;
    receiver_hostname = args[2];
    receiver_port = args[3];

    request = new MemberRequest(message_type);
  } else if (args[1] == "list") {
    if (args.size() != 4) {
      cout << "Invalid parameters count" << endl;
      cout << "./client membership list <member host> <member port>" << endl;
      exit(1);
    }

    message_type = MEMBERSHIP;
    receiver_hostname = args[2];
    receiver_port = args[3];

    request = new MemberRequest(message_type);
  } else {
    cout << "Command not found" << endl;
    cout << "./client membership [join|leave|list]" << endl;
    exit(1);
  }

  return request;
}

SocketMessage* parseSdfsArgs(vector<string> args) {
  if (args.size() <= 1) {
    cout << "Invalid parameters count" << endl;
    cout << "./client sdfs [put|get|delete|ls|store|get-versions]" << endl;
    exit(1);
  }

  SocketMessage* request;

  if (args[1] == "put") {
    string localfilename;
    string sdfsfilename;

    if (args.size() == 4) {
      localfilename = args[2];
      sdfsfilename = args[3];
    } else if (args.size() == 6) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      localfilename = args[4];
      sdfsfilename = args[5];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client sdfs put [hostname] [port] <localfilename> "
              "<sdfsfilename>"
           << endl;
      exit(1);
    }

    request = new FileSystemPutRequest(localfilename, sdfsfilename, 0, true);

  } else if (args[1] == "get") {
    string sdfsfilename;
    string localfilename;

    if (args.size() == 4) {
      sdfsfilename = args[2];
      localfilename = args[3];
    } else if (args.size() == 6) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      sdfsfilename = args[4];
      localfilename = args[5];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client sdfs get [hostname] [port] <sdfsfilename> "
              "<localfilename>"
           << endl;
      exit(1);
    }

    request = new FileSystemGetRequest(sdfsfilename, localfilename, true);

  } else if (args[1] == "delete") {
    string sdfsfilename;

    if (args.size() == 3) {
      sdfsfilename = args[2];
    } else if (args.size() == 5) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      sdfsfilename = args[4];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client sdfs delete [hostname] [port] <sdfsfilename>" << endl;
      exit(1);
    }

    request = new FileSystemDeleteRequest(sdfsfilename, true);

  } else if (args[1] == "ls") {
    string sdfsfilename;

    if (args.size() == 3) {
      sdfsfilename = args[2];
    } else if (args.size() == 5) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      sdfsfilename = args[4];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client sdfs ls [hostname] [port] <sdfsfilename>" << endl;
      exit(1);
    }

    request = new FileSystemLsRequest(sdfsfilename);

  } else if (args[1] == "store") {
    if (args.size() == 4) {
      receiver_hostname = args[2];
      receiver_port = args[3];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client sdfs store [hostname] [port]" << endl;
      exit(1);
    }

    request = new FileSystemStoreRequest();

  } else if (args[1] == "get-versions") {
    string sdfsfilename;
    int num_versions;
    string localfilename;

    if (args.size() == 5) {
      sdfsfilename = args[2];
      num_versions = stoi(args[3]);
      localfilename = args[4];
    } else if (args.size() == 7) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      sdfsfilename = args[4];
      num_versions = stoi(args[5]);
      localfilename = args[6];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client sdfs get-versions [hostname] [port] <sdfsfilename> "
              "<num-versions> <localfilename>"
           << endl;
      exit(1);
    }

    request = new FileSystemGetVersionsRequest(sdfsfilename, num_versions,
                                               localfilename, true);

  } else {
    cout << "Command not found" << endl;
    cout << "./client sdfs [put|get|delete|ls|store|get-versions]" << endl;
    exit(1);
  }

  return request;
}

SocketMessage* parseMlArgs(vector<string> args) {
  if (args.size() <= 2) {
    cout << "Invalid parameters count" << endl;
    cout << "./client ml [add|remove|start|stop|start-phase|stop-phase|status]"
         << endl;
    exit(1);
  }

  SocketMessage* request;

  if (args[1] == "add") {
    string job;
    string model;
    string testset;
    string label;
    int batch_size;

    if (args.size() == 7) {
      job = args[2];
      model = args[3];
      testset = args[4];
      label = args[5];
      batch_size = stoi(args[6]);
    } else if (args.size() == 9) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      job = args[4];
      model = args[5];
      testset = args[6];
      label = args[7];
      batch_size = stoi(args[8]);
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client ml add [hostname] [port] <job> <model> <testset> "
              "<label> <batch_size>"
           << endl;
      exit(1);
    }

    request = new MlAddJobRequest(job, model, testset, label, batch_size);

  } else if (args[1] == "remove") {
    string job;

    if (args.size() == 3) {
      job = args[2];
    } else if (args.size() == 5) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      job = args[4];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client ml remove [hostname] [port] <job> " << endl;
      exit(1);
    }

    request = new MlRemoveJobRequest(job);

  } else if (args[1] == "start") {
    string job;

    if (args.size() == 3) {
      job = args[2];
    } else if (args.size() == 5) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      job = args[4];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client ml start [hostname] [port] <job> " << endl;
      exit(1);
    }

    request = new MlStartJobRequest(job);

  } else if (args[1] == "stop") {
    string job;

    if (args.size() == 3) {
      job = args[2];
    } else if (args.size() == 5) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      job = args[4];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client ml stop [hostname] [port] <job> " << endl;
      exit(1);
    }

    request = new MlStopJobRequest(job);

  } else if (args[1] == "start-phase") {
    string input;
    ML_PHASE phase;

    if (args.size() == 3) {
      input = args[2];
    } else if (args.size() == 5) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      input = args[4];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client ml start-phase [hostname] [port] <training|inference> "
           << endl;
      exit(1);
    }

    if (input == "training") {
      phase = TRAINING;
    } else if (input == "inference") {
      phase = INFERENCE;
    } else {
      cout << "Invalid phase" << endl;
      cout << "./client ml start-phase [hostname] [port] <training|inference> "
           << endl;
      exit(1);
    }

    request = new MlStartPhaseRequest(phase);

  } else if (args[1] == "stop-phase") {
    string input;
    ML_PHASE phase;

    if (args.size() == 3) {
      input = args[2];
    } else if (args.size() == 5) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      input = args[4];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client ml stop-phase [hostname] [port] <training|inference> "
           << endl;
      exit(1);
    }

    if (input == "training") {
      phase = TRAINING;
    } else if (input == "inference") {
      phase = INFERENCE;
    } else {
      cout << "Invalid phase" << endl;
      cout << "./client ml stop-phase [hostname] [port] <training|inference> "
           << endl;
      exit(1);
    }

    request = new MlStopPhaseRequest(phase);

  } else if (args[1] == "status") {
    string job_id;

    if (args.size() == 3) {
      job_id = args[2];
    } else if (args.size() == 5) {
      receiver_hostname = args[2];
      receiver_port = args[3];
      job_id = args[4];
    } else {
      cout << "Invalid parameters count" << endl;
      cout << "./client ml status [hostname] [port] <job_id> " << endl;
      exit(1);
    }

    request = new MlStatusRequest(job_id);

  } else {
    cout << "Command not found" << endl;
    cout << "./client ml [add|remove|start|stop|start-phase|stop-phase|status]"
         << endl;
    exit(1);
  }

  return request;
}

void print_ml_job_status(MlStatusResponse* response) {
  cout << "Latest QPS: " << response->latest_query_rate << endl;

  cout << "Processed query count: " << response->processed_query_count << endl;

  cout << "query processing time: " << endl;
  cout << "\t"
       << "average: " << response->average_query_processing_time << "s" << endl;
  cout << "\t"
       << "standard deviation: "
       << response->std_deviation_query_processing_time << "s" << endl;
  cout << "\t"
       << "median: " << response->median_query_processing_time << "s" << endl;
  cout << "\t"
       << "90th percentile: " << response->percentile_90_query_processing_time
       << "s" << endl;
  cout << "\t"
       << "95th percentile: " << response->percentile_95_query_processing_time
       << "s" << endl;
  cout << "\t"
       << "99th percentile: " << response->percentile_99_query_processing_time
       << "s" << endl;

  cout << "Assigned workers:" << endl;
  if (response->assigned_workers.size() == 0)
    cout << "\t"
         << "No worker" << endl;
  else {
    for (auto worker : response->assigned_workers) {
      cout << "\t" << worker << endl;
    }
  }

  cout << endl;
}
