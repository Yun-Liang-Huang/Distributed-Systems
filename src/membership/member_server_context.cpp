#include "member_server_context.h"

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../utils.h"
#include "member_const.h"
#include "member_utils.h"

using namespace std;

// -------------- generating development data START -------------- //
void create_test_membershiplist() {
  string member1("123456" + MEMBER_ID_DELIMITER + "127.0.0.1" +
                 MEMBER_ID_DELIMITER + "8081");
  string member2("123908" + MEMBER_ID_DELIMITER + "127.0.0.1" +
                 MEMBER_ID_DELIMITER + "8082");
  string member3("129847" + MEMBER_ID_DELIMITER + "127.0.0.1" +
                 MEMBER_ID_DELIMITER + "8083");
  string member4("159308" + MEMBER_ID_DELIMITER + "127.0.0.1" +
                 MEMBER_ID_DELIMITER + "8084");
  string member5("165392" + MEMBER_ID_DELIMITER + "127.0.0.1" +
                 MEMBER_ID_DELIMITER + "8085");
  MemberServerContext::RingMap.insert(member1);
  MemberServerContext::RingMap.insert(member2);
  MemberServerContext::RingMap.insert(member3);
  MemberServerContext::RingMap.insert(member4);
  MemberServerContext::RingMap.insert(member5);
}

// -------------- generating development data END -------------- //

// init static members
MemberServerContext::SERVER_MODE MemberServerContext::ServerMode =
    MemberServerContext::SERVER_MODE::WAIT_INTRODUCER;
string MemberServerContext::Introducer =
    "localhost";  // gethostname(hostname, HOST_NAME_MAX)
string MemberServerContext::Leader;
int MemberServerContext::MemberPort = 8080;
mutex MemberServerContext::RingMapMutex;
set<string> MemberServerContext::RingMap;
unordered_map<string, uint64_t> MemberServerContext::RemovedMember;
string MemberServerContext::SelfMember;
unordered_map<string, int> MemberServerContext::SuspectFailedTime;
vector<string> MemberServerContext::SuccessfulMember;
string MemberServerContext::failureDetectionMessage;

void MemberServerContext::CreateContext(int port) {
  // TODO: init all context here
  // create_test_membershiplist();
  MemberServerContext::MemberPort = port;
  MemberServerContext::CreateContext();
}

void MemberServerContext::CreateContext() {
  MemberServerContext::RingMapMutex.unlock();
  MemberServerContext::RingMapMutex.lock();

  // reset ring map
  MemberServerContext::RingMap = set<string>();
  MemberServerContext::RemovedMember = unordered_map<string, uint64_t>();

  MemberServerContext::SelfMember = MemberUtils::generate_member(
      Utils::getHostName(), MemberServerContext::MemberPort);
  MemberServerContext::RingMap.insert(MemberServerContext::SelfMember);

  MemberServerContext::Leader = MemberServerContext::SelfMember;

  MemberServerContext::ServerMode = SERVER_MODE::WAIT_INTRODUCER;

  MemberServerContext::RingMapMutex.unlock();
}

void MemberServerContext::ReleaseContext() {
  // TODO: release all context here
}
