#ifndef SOCKET_MESSAGE
#define SOCKET_MESSAGE

#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "socket_message_const.h"

using namespace std;

class SocketMessage {
 public:
  SOCKET_MESSAGE_TYPE message_type;

  SocketMessage(){};

  virtual ~SocketMessage(){};

  virtual const string serialize() = 0;

  static string serialize_hashmap(const unordered_map<string, string>&);
  static void deserialize_hashmap(const string hashmap_str,
                                  unordered_map<string, string>&);
  static string serialize_hashmap(const map<int, string>&);
  static void deserialize_hashmap(const string hashmap_str, map<int, string>&);
  static string serialize_vector(const vector<string> vec);
  static void deserialize_vector(const string vector_str,
                                 vector<string>& result);

  static string serialize_hashset(const unordered_set<string> set);
  static void deserialize_hashset(const string set_str,
                                  unordered_set<string>& result);
  static string serialize_hashset(const unordered_set<int> set);
  static void deserialize_hashset(const string set_str,
                                  unordered_set<int>& result);
  static string serialize_set(const set<int> set);
  static void deserialize_set(const string set_str, set<int>& result);

  static string serialize_set(const set<string> set);
  static void deserialize_set(const string set_str, set<string>& result);

 private:
  virtual void deserialize(const char*) = 0;
};

#endif