#include <gtest/gtest.h>

#include "../src/client_request_sender.h"
#include "../src/query_log_response.h"
#include "../src/query_request.h"
#include "../src/utils.h"

char* address = "localhost";
int port = 8088;

TEST(QueryLogTest, FrequentPattern) {
  std::string query = "Firefox/3.6.5";
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::STRING_SEARCH, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -c '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, RarePattern) {
  std::string query =
      "13/Dec/2023:01:14";  // only happens twice, in vm5 and vm10
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::STRING_SEARCH, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -c '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, SomewhatFrequent) {
  std::string query = "\"POST /list HTTP/1.0\" 200 4986";
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::STRING_SEARCH, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -c '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, OccurInOneLog) {
  std::string query = "201.205.194.108";
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::STRING_SEARCH, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -c '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, OccurInSomeLogs) {  // same as RarePattern
  std::string query =
      "13/Dec/2023:01:14";  // only happens twice, in vm5 and vm10
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::STRING_SEARCH, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -c '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, OccurInAllLogs) {
  std::string query = "13/Dec/2023:01";
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::STRING_SEARCH, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -c '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, Regex) {
  std::string query = "Mac OS.*Firefox";
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::REGULAR_EXPRESSION, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -Ec '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, SomewhatFrequentRegex) {
  std::string query = "^124.*";
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::REGULAR_EXPRESSION, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -Ec '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, RareFrequentRegex) {
  std::string query = "GET.*5023.*Mozilla.*Mac OS X 10_5_7.*Chrome";
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::REGULAR_EXPRESSION, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -Ec '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}

TEST(QueryLogTest, FrequentRegex) {
  std::string query = "[a-z]{6}V*.[0-9]";
  QueryRequest* request =
      new QueryRequest(SOCKET_MESSAGE_TYPE::QUERY_LOG,
                       QUERY_TYPE::REGULAR_EXPRESSION, query, ".*.log");

  QueryLogResponse* response =
      (QueryLogResponse*)RequestSender::send_request(address, port, request);

  std::string golden_grep_result =
      Utils::exec(("grep -Ec '" + query +
                   "' logs/*.log | awk -F: '{ s+=$2 } END { print s }'")
                      .c_str());
  int match_line_count = stoi(golden_grep_result);

  EXPECT_EQ(response->match_line_count, match_line_count);

  delete request;
  delete response;
}