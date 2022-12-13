#ifndef MEMBER_SERVER_CONTEXT
#define MEMBER_SERVER_CONTEXT

#include <array>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "member_api_handler.h"
#include "member_const.h"

using namespace std;

class MemberServerContext {
 public:
  enum SERVER_MODE {
    WAIT_INTRODUCER,  // will drop all UDP traffic until introduce invite
                      // current server (by TCP API) to the membership group

    ACTIVE_MEMBER,  // will respond UDP traffic normally
  };

  static MemberServerContext::SERVER_MODE ServerMode;
  static mutex RingMapMutex;
  static set<string> RingMap;
  static unordered_map<string, string> PreviousRingMap;
  static unordered_map<string, uint64_t> RemovedMember;  // member:removed time
  static string Introducer;
  static string Leader;
  static int MemberPort;
  static string SelfMember;
  static unordered_map<string, int> SuspectFailedTime;
  static vector<string> SuccessfulMember;
  static string failureDetectionMessage;
  static const int SUCCESSOR_NUMBER = 3;

  static void CreateContext(int);
  static void CreateContext();
  static void ReleaseContext();
};

#endif