
#include "socket_message.h"

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

string SocketMessage::serialize_hashmap(
    const unordered_map<string, string>& hashmap) {
  ostringstream oss(ostringstream::ate);
  for (auto& [key, value] : hashmap) {
    oss << key.size() << ":" << key;
    oss << value.size() << ":" << value;
  }
  return oss.str();
}

void SocketMessage::deserialize_hashmap(const string hashmap_str,
                                        unordered_map<string, string>& result) {
  int start = 0;
  vector<string> tokens;
  try {
    while (start < hashmap_str.size()) {
      int end = hashmap_str.find(":", start);
      int token_len = stoi(hashmap_str.substr(start, end - start));
      start = end + 1;

      string token = hashmap_str.substr(start, token_len);
      tokens.push_back(token);

      start += token_len;
    }

    // assert(tokens.size() % 2 == 0);
  } catch (exception e) {
    throw runtime_error("Deserializing hashmap error, input: " + hashmap_str);
  }

  for (int i = 0; i < tokens.size(); i += 2) {
    string key = tokens[i];
    string value = tokens[i + 1];
    result[key] = value;
  }
}

string SocketMessage::serialize_hashmap(const map<int, string>& hashmap) {
  ostringstream oss(ostringstream::ate);
  for (auto& [key, value] : hashmap) {
    oss << to_string(key).size() << ":" << key;
    oss << value.size() << ":" << value;
  }
  return oss.str();
}

void SocketMessage::deserialize_hashmap(const string hashmap_str,
                                        map<int, string>& result) {
  int start = 0;
  vector<string> tokens;
  try {
    while (start < hashmap_str.size()) {
      int end = hashmap_str.find(":", start);
      int token_len = stoi(hashmap_str.substr(start, end - start));
      start = end + 1;

      string token = hashmap_str.substr(start, token_len);
      tokens.push_back(token);

      start += token_len;
    }

    // assert(tokens.size() % 2 == 0);
  } catch (exception e) {
    throw runtime_error("Deserializing hashmap error, input: " + hashmap_str);
  }

  for (int i = 0; i < tokens.size(); i += 2) {
    int key = stoi(tokens[i]);
    string value = tokens[i + 1];
    result[key] = value;
  }
}

string SocketMessage::serialize_vector(const vector<string> vec) {
  ostringstream oss(ostringstream::ate);
  for (auto value : vec) {
    oss << value.size() << ":" << value;
  }
  return oss.str();
}

void SocketMessage::deserialize_vector(const string vector_str,
                                       vector<string>& result) {
  int start = 0;
  try {
    while (start < vector_str.size()) {
      int end = vector_str.find(":", start);
      int token_len = stoi(vector_str.substr(start, end - start));
      start = end + 1;

      string token = vector_str.substr(start, token_len);
      result.push_back(token);

      start += token_len;
    }
  } catch (exception e) {
    throw runtime_error("Deserializing vector error, input: " + vector_str);
  }
}

string SocketMessage::serialize_hashset(const unordered_set<string> set) {
  ostringstream oss(ostringstream::ate);
  for (auto value : set) {
    oss << value.size() << ":" << value;
  }
  return oss.str();
}
void SocketMessage::deserialize_hashset(const string set_str,
                                        unordered_set<string>& result) {
  int start = 0;
  try {
    while (start < set_str.size()) {
      int end = set_str.find(":", start);
      int token_len = stoi(set_str.substr(start, end - start));
      start = end + 1;

      string token = set_str.substr(start, token_len);
      result.insert(token);

      start += token_len;
    }
  } catch (exception e) {
    throw runtime_error("Deserializing set error, input: " + set_str);
  }
}

string SocketMessage::serialize_hashset(const unordered_set<int> set) {
  ostringstream oss(ostringstream::ate);
  for (auto value : set) {
    string str_value = to_string(value);
    oss << str_value.size() << ":" << str_value;
  }
  return oss.str();
}
void SocketMessage::deserialize_hashset(const string set_str,
                                        unordered_set<int>& result) {
  int start = 0;
  try {
    while (start < set_str.size()) {
      int end = set_str.find(":", start);
      int token_len = stoi(set_str.substr(start, end - start));
      start = end + 1;

      string token = set_str.substr(start, token_len);
      result.insert(stoi(token));

      start += token_len;
    }
  } catch (exception e) {
    throw runtime_error("Deserializing set error, input: " + set_str);
  }
}

string SocketMessage::serialize_set(const set<int> set) {
  ostringstream oss(ostringstream::ate);
  for (auto value : set) {
    string str_value = to_string(value);
    oss << str_value.size() << ":" << str_value;
  }
  return oss.str();
}
void SocketMessage::deserialize_set(const string set_str, set<int>& result) {
  int start = 0;
  try {
    while (start < set_str.size()) {
      int end = set_str.find(":", start);
      int token_len = stoi(set_str.substr(start, end - start));
      start = end + 1;

      string token = set_str.substr(start, token_len);
      result.insert(stoi(token));

      start += token_len;
    }
  } catch (exception e) {
    throw runtime_error("Deserializing set error, input: " + set_str);
  }
}

string SocketMessage::serialize_set(const set<string> set) {
  ostringstream oss(ostringstream::ate);
  for (auto value : set) {
    oss << value.size() << ":" << value;
  }
  return oss.str();
}
void SocketMessage::deserialize_set(const string set_str, set<string>& result) {
  int start = 0;
  try {
    while (start < set_str.size()) {
      int end = set_str.find(":", start);
      int token_len = stoi(set_str.substr(start, end - start));
      start = end + 1;

      string token = set_str.substr(start, token_len);
      result.insert(token);

      start += token_len;
    }
  } catch (exception e) {
    throw runtime_error("Deserializing set error, input: " + set_str);
  }
}