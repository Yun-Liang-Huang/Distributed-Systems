#include "query_api_handler.h"

#include <dirent.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <future>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

#include "../client_request_sender.h"
#include "../utils.h"
#include "query_log_response.h"
#include "query_request.h"

using namespace std;

#define PORT 8088

string format_log(string header, string logs) {
  istringstream iss(logs);
  ostringstream oss(ostringstream::ate);
  string log;
  while (getline(iss, log)) {
    oss << "[" << header << "] " << log << endl;
  }

  return oss.str();
}

QueryLogResponse* QueryApiHandler::queryLog(QUERY_TYPE query_type, string input,
                                            string file_pattern) {
  QueryRequest* request =
      new QueryRequest(QUERY_WORKER_LOG, query_type, input, file_pattern);

  vector<pair<string, int>> workers;
  workers.push_back(make_pair("fa22-cs425-0401.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0402.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0403.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0404.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0405.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0406.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0407.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0408.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0409.cs.illinois.edu", 8088));
  workers.push_back(make_pair("fa22-cs425-0410.cs.illinois.edu", 8088));

  // workers.push_back(make_pair("localhost", 8088));
  // workers.push_back(make_pair("localhost", 8087));

  vector<string> log_list;
  int match_line_count = 0;
  int total_line_count = 0;
  unordered_map<string, future<QueryLogResponse*>> futures;
  vector<thread> thread_pool;

  auto request_handler = [&](promise<QueryLogResponse*> promise, string host,
                             int port, QueryRequest* request) {
    try {
      QueryLogResponse* response =
          (QueryLogResponse*)RequestSender::send_request(host.c_str(), port,
                                                         request);

      promise.set_value(response);
    } catch (exception& e) {
      cout << e.what() << endl;
      promise.set_value(nullptr);
    }
  };

  // setup promise, future, and threads for each worker
  for (auto worker : workers) {
    promise<QueryLogResponse*> promise;
    // futures.push_back(promise.get_future());
    futures[worker.first + ":" + to_string(worker.second)] =
        promise.get_future();
    thread thread(request_handler, move(promise), worker.first, worker.second,
                  request);
    thread_pool.push_back(move(thread));
  }

  // start all threads and wait for all
  for (auto&& thread : thread_pool) {
    thread.join();
  }

  // aggregate results
  for (auto& [header, future] : futures) {
    QueryLogResponse* response = future.get();
    if (response) {
      log_list.push_back(format_log(header, response->logs));
      match_line_count += response->match_line_count;
      total_line_count += response->total_line_count;
      delete response;
    }
  }

  cout << "Coordinator match line count: " << match_line_count << endl;
  cout << "Coordinator total line count: " << total_line_count << endl;

  delete request;
  string logs = Utils::join(log_list, "\n");
  QueryLogResponse* query_response =
      new QueryLogResponse(logs, match_line_count, total_line_count);

  cout << "query log response len: " << query_response->logs.size() << endl;

  return query_response;
}

QueryLogResponse* QueryApiHandler::queryWorkerLog(QUERY_TYPE query_type,
                                                  string input,
                                                  string file_pattern) {
  string output;
  int match_line_count = 0;
  int total_line_count = 0;
  FILE* ptr;
  Utils util;
  regex file_regex(file_pattern);
  regex str_regex(input);
  // Transform input to uppercase if case insensitive search
  if (query_type == CASE_INSENSITIVE) {
    transform(input.begin(), input.end(), input.begin(), ::toupper);
  }

  // Locate Current Work Directory
  char cwd[1024];
  getcwd(cwd, 1024);
  DIR* directory = opendir(cwd);
  if (NULL == directory) {
    cout << "Directory is empty" << endl;
  }

  struct dirent* entry;
  // Loop through all files under current work directory
  while (NULL != (entry = readdir(directory))) {
    if (regex_match(entry->d_name, file_regex)) {
      ptr = fopen(entry->d_name, "r");
      if (NULL == ptr) {
        cout << "File cannot be opened: " << entry->d_name << endl;
      }

      char* buffer = NULL;
      size_t buffer_size = 0;

      // Regular expression calls linux command due to performance issue
      if (REGULAR_EXPRESSION == query_type) {
        string grep("grep -E \"" + input + "\" \"" + entry->d_name + "\"");
        string result = util.exec(grep.c_str());
        output = format_log(entry->d_name, result);

        // Grep match count
        string grep_cnt("grep -Ec \"" + input + "\" \"" + entry->d_name + "\"");
        match_line_count = stoi(util.exec(grep_cnt.c_str()));
      }

      while (getline(&buffer, &buffer_size, ptr) >= 0) {
        total_line_count += 1;
        string str(buffer);
        if (STRING_SEARCH == query_type) {
          size_t found = str.find(input);
          if (found != string::npos) {
            output += format_log(entry->d_name, str);
            match_line_count += 1;
          }
        } else if (CASE_INSENSITIVE == query_type) {
          string temp(str);
          transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
          size_t found = temp.find(input);
          if (found != string::npos) {
            output += format_log(entry->d_name, str);
            match_line_count += 1;
          }
        }
      }

      free(buffer);
      buffer = NULL;
      fclose(ptr);
    }
  }

  cout << "Output size: " << output.size() << endl;
  cout << "Output match_line_count: " << match_line_count << endl;
  cout << "Output total_line_count: " << total_line_count << endl;

  return new QueryLogResponse(string(output), match_line_count,
                              total_line_count);
}

SocketMessage* QueryApiHandler::api_handler(const char* buffer) {
  SocketMessage* output;
  QueryRequest* request = new QueryRequest(buffer);

  if (QUERY_LOG == request->message_type) {
    output =
        queryLog(request->query_type, request->input, request->file_pattern);
  } else if (QUERY_WORKER_LOG == request->message_type) {
    output = queryWorkerLog(request->query_type, request->input,
                            request->file_pattern);
  } else {
    cout << "Request not found" << endl;
  }

  cout << "Result: " << output << endl;

  return output;
}
