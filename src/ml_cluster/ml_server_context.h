#ifndef ML_SERVER_CONTEXT
#define ML_SERVER_CONTEXT

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../socket_message.h"
#include "../utils.h"
// #include "../socket_message_utils.h"

using namespace std;

enum JOB_STATUS {
  RUNNING,
  IDLE,
  DONE,
};

class JobMetadata {
 public:
  JOB_STATUS status;
  string model;
  string testset;
  string label;
  int batch_size;

  JobMetadata() {}

  JobMetadata(JOB_STATUS status, string model, string testset, string label,
              int batch_size)
      : status(status),
        model(model),
        testset(testset),
        label(label),
        batch_size(batch_size) {}

  string serialize() {
    unordered_map<string, string> payload;
    payload["0"] = to_string(status);
    payload["1"] = model;
    payload["2"] = testset;
    payload["3"] = label;
    payload["4"] = to_string(batch_size);
    return SocketMessage::serialize_hashmap(payload);
  }

  static JobMetadata* deserialize(string data_string) {
    unordered_map<string, string> payload;
    SocketMessage::deserialize_hashmap(data_string, payload);
    JobMetadata* data =
        new JobMetadata((JOB_STATUS)stoi(payload["0"]), payload["1"],
                        payload["2"], payload["3"], stoi(payload["4"]));
    return data;
  }

  string get_status() {
    switch (status) {
      case RUNNING:
        return "RUNNING";
      case IDLE:
        return "IDLE";
      case DONE:
        return "DONE";
    }
  }
};

class SubJob {
 public:
  int offset;
  int limit;
  string output_filename;
  double start_time = 0;
  double end_time = 0;

  SubJob() {}
  SubJob(int offset, int limit, string output_filename)
      : offset(offset), limit(limit), output_filename(output_filename) {}

  string serialize() {
    unordered_map<string, string> payload;
    payload["0"] = to_string(offset);
    payload["1"] = to_string(limit);
    payload["2"] = output_filename;
    payload["3"] = to_string(start_time);
    payload["4"] = to_string(end_time);
    return SocketMessage::serialize_hashmap(payload);
  }

  static SubJob* deserialize(string data_string) {
    unordered_map<string, string> payload;
    SocketMessage::deserialize_hashmap(data_string, payload);
    SubJob* data =
        new SubJob(stoi(payload["0"]), stoi(payload["1"]), payload["2"]);
    data->start_time = stoull(payload["3"]);
    data->end_time = stoull(payload["4"]);
    return data;
  }

  double get_processed_time() {
    if (start_time == 0 || end_time == 0) return 0;  // time is not set
    return end_time - start_time;
  }
  void set_start_time() { start_time = Utils::get_time_second(); }
  void set_end_time() { end_time = Utils::get_time_second(); }
  void reset_time() {
    start_time = 0;
    end_time = 0;
  }
};

class JobStatus {
 public:
  double last_start_time;
  double total_running_seconds;
  int accurate_query_count;
  int finished_query_count;
  int running_worker_count;
  unordered_map<string, SubJob*> running_subjobs;  // vm_id : SubJob
  queue<SubJob*> todo_subjobs;
  set<SubJob*> finished_subjobs;

  JobStatus();
  static void start_job(JobStatus* jobStatus);
  static void stop_job(JobStatus* jobStatus);

  string serialize() {
    // packing running_subjobs
    unordered_map<string, string> running_subjobs_map;
    for (auto const& [key, value] : running_subjobs) {
      running_subjobs_map[key] = value->serialize();
    }

    unordered_map<string, string> payload;
    payload["0"] = to_string(last_start_time);
    payload["1"] = to_string(total_running_seconds);
    payload["2"] = to_string(accurate_query_count);
    payload["3"] = to_string(finished_query_count);
    payload["4"] = to_string(running_worker_count);
    payload["5"] = SocketMessage::serialize_hashmap(running_subjobs_map);
    payload["6"] = JobStatus::serialize_queue(todo_subjobs);
    payload["7"] = JobStatus::serialize_set(finished_subjobs);
    return SocketMessage::serialize_hashmap(payload);
  }

  static JobStatus* deserialize(string data_string) {
    unordered_map<string, string> payload;
    JobStatus* data = new JobStatus();
    SocketMessage::deserialize_hashmap(data_string, payload);
    data->last_start_time = stod(payload["0"]);
    data->total_running_seconds = stod(payload["1"]);
    data->accurate_query_count = stoi(payload["2"]);
    data->finished_query_count = stoi(payload["3"]);
    data->running_worker_count = stoi(payload["4"]);

    // unpacking running_subjobs
    unordered_map<string, string> running_subjobs_map;
    SocketMessage::deserialize_hashmap(payload["5"], running_subjobs_map);
    for (auto const& [key, value] : running_subjobs_map) {
      data->running_subjobs[key] = SubJob::deserialize(value);
    }

    JobStatus::deserialize_queue(payload["6"], data->todo_subjobs);
    JobStatus::deserialize_set(payload["7"], data->finished_subjobs);
    return data;
  }

  static string serialize_queue(queue<SubJob*> queue) {
    ostringstream oss(ostringstream::ate);
    int size = queue.size();
    for (int i = 0; i < size; i++) {
      SubJob* subJob = queue.front();
      queue.pop();
      oss << subJob->serialize().size() << ":" << subJob->serialize();
    }
    return oss.str();
  }

  static void deserialize_queue(const string queue_str,
                                queue<SubJob*>& result) {
    int start = 0;
    try {
      while (start < queue_str.size()) {
        int end = queue_str.find(":", start);
        int token_len = stoi(queue_str.substr(start, end - start));
        start = end + 1;

        string token = queue_str.substr(start, token_len);
        SubJob* subJob = SubJob::deserialize(token);
        result.push(subJob);

        start += token_len;
      }
    } catch (exception e) {
      throw runtime_error("Deserializing queue error, input: " + queue_str);
    }
  }

  static string serialize_set(const set<SubJob*> set) {
    ostringstream oss(ostringstream::ate);
    for (auto value : set) {
      oss << value->serialize().size() << ":" << value->serialize();
    }
    return oss.str();
  }

  static void deserialize_set(const string set_str, set<SubJob*>& result) {
    int start = 0;
    try {
      while (start < set_str.size()) {
        int end = set_str.find(":", start);
        int token_len = stoi(set_str.substr(start, end - start));
        start = end + 1;

        string token = set_str.substr(start, token_len);
        SubJob* subJob = SubJob::deserialize(token);
        result.insert(subJob);

        start += token_len;
      }
    } catch (exception e) {
      throw runtime_error("Deserializing set error, input: " + set_str);
    }
  }
};

class MlServerContext {
 public:
  static unordered_map<string, JobMetadata*> JobMetadataMap;
  static unordered_map<string, JobStatus*> JobStatusMap;

  static queue<string> worker_pool;
  static set<string> all_workers;

  static int SequenceNumber;

  static mutex GlobalMutex;
  static mutex LocalMutex;
  static mutex InferenceResourceMutex;

  static void CreateContext();
  static void ReleaseContext();
};

#endif