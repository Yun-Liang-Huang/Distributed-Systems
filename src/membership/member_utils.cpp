#include "member_utils.h"

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

Member::Member(string hostname, int port) {
  long localtime = (chrono::system_clock::now().time_since_epoch()).count();
  string localtime_str = to_string(localtime);
  this->time_id = localtime_str.substr(localtime_str.size() - ID_LENGTH);

  this->port = to_string(port);

  this->hostname = hostname;
}

Member::Member(string member_id) {
  vector<string> tokens;
  size_t pos = 0;
  while ((pos = member_id.find(MEMBER_ID_DELIMITER)) != std::string::npos) {
    string token(member_id.substr(0, pos));
    tokens.push_back(token);
    member_id.erase(0, pos + MEMBER_ID_DELIMITER.length());
  }
  if (tokens.size() == 2) {
    port = member_id;  // input string remains port
    time_id = tokens[0];
    hostname = tokens.back();
  } else {
    throw runtime_error("Error occurs when parsing member ID");
  }
}

string Member::to_member_id() {
  return time_id + MEMBER_ID_DELIMITER + hostname + MEMBER_ID_DELIMITER + port;
}

bool MemberUtils::diffMap(const unordered_map<string, string>& olderMap,
                          const unordered_map<string, string>& newerMap,
                          unordered_map<string, string>& diffMap) {
  for (auto const& [key, value] : newerMap) {
    // insert keys that are new or with different value
    if (olderMap.count(key) == 0 || olderMap.at(key) != value) {
      diffMap[key] = value;
    }
  }

  for (auto const& [key, value] : olderMap) {
    // remove keys that are not in newer map
    if (newerMap.count(key) == 0) {
      diffMap[key] = KEY_REMOVED;
    }
  }
  return diffMap.size() > 0;
}

bool MemberUtils::diffMap(const unordered_map<string, string>& olderMap,
                          const unordered_map<string, string>& newerMap) {
  unordered_map<string, string> diffMap;
  return MemberUtils::diffMap(olderMap, newerMap, diffMap);
}

string MemberUtils::generate_member(string host, int port) {
  long localtime = (chrono::system_clock::now().time_since_epoch()).count();
  string localtime_str = to_string(localtime);
  string localtime_id = localtime_str.substr(localtime_str.size() - ID_LENGTH);
  return localtime_id + MEMBER_ID_DELIMITER + host + MEMBER_ID_DELIMITER +
         to_string(port);
}

// Membership List
template <typename T, typename U>
void MemberUtils::printMemberList(string listname, unordered_map<T, U> map) {
  MemberServerContext::RingMapMutex.lock();

  cout << "=========== " << listname << " ===========" << endl;
  cout << "Member number: " << map.size() << endl;
  for (auto const& [key, value] : map) {
    string marker = (key == MemberServerContext::Leader) ? " *" : "";
    cout << "\tMember: " << key << marker << std::endl;
    // cout << "\t\t->" << value << std::endl;
  }
  cout << endl;

  MemberServerContext::RingMapMutex.unlock();
}

template <typename T>
void MemberUtils::printMemberList(string listname, set<T> set) {
  MemberServerContext::RingMapMutex.lock();

  cout << "=========== " << listname << " ===========" << endl;
  cout << "Member number: " << set.size() << endl;
  for (auto member : set) {
    string marker = (member == MemberServerContext::Leader) ? " *" : "";
    cout << "\tMember: " << member << marker << std::endl;
    // cout << "\t\t->" << value << std::endl;
  }
  cout << endl;

  MemberServerContext::RingMapMutex.unlock();
}

void MemberUtils::printMembershipList() {
  MemberUtils::printMemberList("Membership List", MemberServerContext::RingMap);

  MemberUtils::printMemberList("Removed Member List",
                               MemberServerContext::RemovedMember);
}

void MemberUtils::getNRandomMembers(set<string> source_members, int n,
                                    unordered_set<string>& base_members,
                                    unordered_set<string>& result) {
  vector<string> members(source_members.begin(), source_members.end());

  // randomize the member list
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  auto rng = std::default_random_engine{seed};
  std::shuffle(std::begin(members), std::end(members), rng);

  int random_member_headcount = n - base_members.size();

  for (auto member : members) {
    if (base_members.count(member) != 0) {
      // always include member if it is a base member
      result.insert(member);
    } else if (random_member_headcount > 0) {
      // member isn't a base member, but still have headcount
      result.insert(member);
      random_member_headcount -= 1;
    }
  }
}

Member MemberUtils::parse_member_id(string member_id) {
  return Member(member_id);
}