#ifndef FILESYSTEM_SERVER_CONTEXT
#define FILESYSTEM_SERVER_CONTEXT

#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../socket_message.h"

using namespace std;

class FileMetadata {
 public:
  unordered_set<string> replica_ids;  // set of replica server_ids
  unordered_set<int> committed_versions;

  string serialize() {
    unordered_map<string, string> payload;
    payload["0"] = SocketMessage::serialize_hashset(replica_ids);
    payload["1"] = SocketMessage::serialize_hashset(committed_versions);
    return SocketMessage::serialize_hashmap(payload);
  }

  static FileMetadata* deserialize(string data_string) {
    unordered_map<string, string> payload;
    FileMetadata* data = new FileMetadata();
    SocketMessage::deserialize_hashmap(data_string, payload);
    SocketMessage::deserialize_hashset(payload["0"], data->replica_ids);
    SocketMessage::deserialize_hashset(payload["1"], data->committed_versions);
    return data;
  }
};

class FileVersionMetadata {
 public:
  map<int, string> version_filenames;

  string serialize() {
    unordered_map<string, string> payload;
    payload["0"] = SocketMessage::serialize_hashmap(version_filenames);
    return SocketMessage::serialize_hashmap(payload);
  }

  static FileVersionMetadata* deserialize(string data_string) {
    unordered_map<string, string> payload;
    FileVersionMetadata* data = new FileVersionMetadata();
    SocketMessage::deserialize_hashmap(data_string, payload);
    SocketMessage::deserialize_hashmap(payload["0"], data->version_filenames);
    return data;
  }
};

class FileSystemServerContext {
 public:
  static const int REPLICA_NUMBER = 4;

  static unordered_map<string, FileMetadata*> FileMetadataMap;
  static unordered_map<string, FileVersionMetadata*> FileMap;
  static unordered_set<string> RecoveredFailureServers;

  static int SequenceNumber;

  static mutex GlobalMutex;
  static mutex LocalMutex;

  static void CreateContext();
  static void ReleaseContext();
};

#endif