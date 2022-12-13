#ifndef MEMBER_CONST
#define MEMBER_CONST

#include <iostream>

using namespace std;

const string MEMBER_ID_DELIMITER = ":";
const int TIME_OFFSET = 1000000;
// Mac: microseconds
// const uint64_t PERIODIC_PING_CYCLE = 2000000 / TIME_OFFSET;
// const int FAILURE_TIMEOUT = 5000000 / TIME_OFFSET;
// Linux: nanoseconds
const uint64_t PERIODIC_PING_CYCLE = 1000000000 / TIME_OFFSET;
const int FAILURE_TIMEOUT = 5000000000 / TIME_OFFSET;

#endif
