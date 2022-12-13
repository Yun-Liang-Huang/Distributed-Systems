#ifndef MEMBER_UTILS
#define MEMBER_UTILS

#include <algorithm>
#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "member_const.h"
#include "member_server_context.h"

using namespace std;

const string KEY_REMOVED = "Key is removed";
const int ID_LENGTH = 4;
class Member {
 public:
  string time_id;
  string hostname;
  string port;

  Member() {}
  Member(string hostname, int port);

  Member(string member_id);

  string to_member_id();
};

class MemberUtils {
 public:
  static bool diffMap(const unordered_map<string, string>& olderMap,
                      const unordered_map<string, string>& newerMap,
                      unordered_map<string, string>& diffMap);

  static bool diffMap(const unordered_map<string, string>& olderMap,
                      const unordered_map<string, string>& newerMap);

  static string generate_member(string host, int port);

  // Membership List
  template <typename T, typename U>
  static void printMemberList(string listname, unordered_map<T, U> map);

  template <typename T>
  static void printMemberList(string listname, set<T> set);

  static void printMembershipList();

  static void getNRandomMembers(set<string> source_members, int n,
                                unordered_set<string>& base_members,
                                unordered_set<string>& result);

  static Member parse_member_id(string member_id);
};

#endif