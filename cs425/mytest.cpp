#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> /* srand, rand */
#include <string.h>
#include <sys/socket.h>
#include <time.h> /* time */

#include <cassert>
#include <iostream>
#include <sstream>
#include <thread>

#include "src/filesystem/filesystem_server_context.h"
#include "src/filesystem/filesystem_utils.h"
#include "src/membership/member_server_context.h"
#include "src/membership/member_traffic_monitor.h"
#include "src/membership/member_utils.h"
#include "src/ml_cluster/ml_worker_utils.h"
#include "src/socket_message_utils.h"
#include "src/tcp_service.h"
#include "src/utils.h"

using namespace std;

#define PORT 8088

void test_random_member() {
  set<string> members = {"a", "b", "c", "d"};
  unordered_set<string> base_members = {"a", "d"};
  int n = 3;
  int rounds = 10000;
  unordered_map<string, int> counts;

  for (int i = 0; i < rounds; i++) {
    unordered_set<string> result;

    MemberUtils::getNRandomMembers(members, n, base_members, result);

    for (auto value : result) {
      // cout << value << " ";
      if (counts.count(value) == 0) {
        counts[value] = 0;
      }
      counts[value] += 1;
    }

    // cout << endl;
  }

  for (auto& [key, value] : counts) {
    // cout << "key: " << key << ", value: " << value << endl;
  }

  assert(counts["b"] + counts["c"] == rounds);
  assert(counts["a"] == rounds);
  assert(counts["d"] == rounds);

  cout << "PASS test_random_member" << endl;
}

void test_metadata_serialization() {
  FileMetadata* data1 = new FileMetadata();
  data1->replica_ids.insert("A");
  data1->replica_ids.insert("B");
  data1->replica_ids.insert("C");
  data1->committed_versions.insert(123);

  string serialized_data = data1->serialize();
  FileMetadata* data2 = FileMetadata::deserialize(serialized_data);

  cout << data2->serialize() << endl;
}

void test_update_filesystem_status() {
  FileSystemServerContext::CreateContext();
  MemberServerContext::CreateContext();
  MemberServerContext::Leader = "leader";
  assert(FileSystemServerContext::FileMetadataMap.size() == 0);
  assert(FileSystemServerContext::SequenceNumber == 0);
  assert(FileSystemServerContext::FileMap.size() == 0);
  assert(FileSystemServerContext::RecoveredFailureServers.size() == 0);

  string filename1 = "abcd";
  string filename2 = "zzzz";
  int seq_number = 123;

  FileSystemServerContext::FileMap[filename1] = new FileVersionMetadata();
  FileSystemServerContext::FileMap[filename2] = new FileVersionMetadata();

  unordered_map<string, FileMetadata*> test_metadata;
  test_metadata[filename1] = new FileMetadata();
  test_metadata[filename1]->committed_versions.insert(seq_number);
  test_metadata[filename2] = new FileMetadata();

  unordered_set<string> test_recovered_servers = {"server 1", "server 2"};

  FileSystemUtils::update_filesystem_status(10, test_metadata,
                                            test_recovered_servers);

  assert(FileSystemServerContext::SequenceNumber == 10);
  assert(FileSystemServerContext::FileMetadataMap.size() == 2);
  assert(FileSystemServerContext::FileMetadataMap[filename1]
             ->committed_versions.count(seq_number) > 0);
  assert(FileSystemServerContext::FileMetadataMap[filename2]
             ->committed_versions.count(seq_number) == 0);
  assert(FileSystemServerContext::RecoveredFailureServers.size() == 2);
  assert(FileSystemServerContext::RecoveredFailureServers.count("server 1") >
         0);
  assert(FileSystemServerContext::RecoveredFailureServers.count(
             "server not exists") == 0);
  cout << "PASS test_update_filesystem_status" << endl;
}

void test_collect_replicating_files() {
  unordered_set<string> failure_servers;
  unordered_set<string> todo_files;

  FileSystemServerContext::CreateContext();
  string filename1 = "filename1";
  string filename2 = "filename2";
  string server1 = "server1";
  string server2 = "server2";
  string server3 = "server3";
  string server4 = "server4";

  FileSystemServerContext::FileMetadataMap[filename1] = new FileMetadata();
  FileSystemServerContext::FileMetadataMap[filename1]->replica_ids.insert(
      server1);
  FileSystemServerContext::FileMetadataMap[filename1]->replica_ids.insert(
      server2);
  FileSystemServerContext::FileMetadataMap[filename1]->replica_ids.insert(
      server3);
  FileSystemServerContext::FileMetadataMap[filename1]->replica_ids.insert(
      server4);

  FileSystemServerContext::FileMetadataMap[filename2] = new FileMetadata();
  FileSystemServerContext::FileMetadataMap[filename2]->replica_ids.insert(
      server2);

  failure_servers.insert(server1);
  // failure_servers.insert(server3);

  FileSystemUtils::collect_replicating_files(failure_servers, todo_files);

  assert(todo_files.count(filename1) == 1);
  assert(todo_files.count(filename2) == 0);
  cout << "PASS test_collect_replicating_files" << endl;
}

void test_execute_python_script() {
  string model_name = "src/ml_cluster/ml_scripts/vit_model.py";
  string input_filename = "src/ml_cluster/ml_scripts/vit_test_set_small.txt";
  string output_filename = "test_vit_output.txt";
  MlWorkerUtils::execute_python_script(model_name, input_filename,
                                       output_filename);

  std::ifstream output_file(output_filename);
  int line_count = std::count(std::istreambuf_iterator<char>(output_file),
                              std::istreambuf_iterator<char>(), '\n');
  assert(line_count == 10);
  cout << "PASS test_execute_python_script" << endl;
}

void test_crop_file() {
  string input_filename = "src/ml_cluster/ml_scripts/vit_test_set.txt";
  string output_filename = "temp.output";
  int start = 1;
  int end = 15;

  Utils::crop_file(input_filename, output_filename, 1, 15);

  std::ifstream output_file(output_filename);
  int line_count = std::count(std::istreambuf_iterator<char>(output_file),
                              std::istreambuf_iterator<char>(), '\n');
  assert(line_count == end - start + 1);
  std::remove(output_filename.c_str());
  cout << "PASS test_crop_file" << endl;
}

int main(int argc, char const* argv[]) {
  test_random_member();
  test_update_filesystem_status();
  test_collect_replicating_files();
  // test_execute_python_script();
  test_crop_file();
  return 0;
}
