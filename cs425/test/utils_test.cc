#include <gtest/gtest.h>

#include "../src/member_utils.h"

using namespace std;

TEST(UtilsTest, MapDiff0) {
  unordered_map<string, string> map({{"A", "B"}, {"B", "C"}, {"C", "A"}});
  unordered_map<string, string> diffMap;

  EXPECT_FALSE(MemberUtils::diffMap(map, map, diffMap));
  EXPECT_FALSE(MemberUtils::diffMap(map, map));
}

TEST(UtilsTest, MapDiff1) {
  unordered_map<string, string> olderMap({{"A", "B"}, {"B", "C"}, {"C", "A"}});
  unordered_map<string, string> newerMap(
      {{"A", "B"}, {"B", "C"}, {"C", "D"}, {"D", "A"}});
  unordered_map<string, string> diffMap;

  bool hasDiff = MemberUtils::diffMap(olderMap, newerMap, diffMap);

  EXPECT_TRUE(hasDiff);
  EXPECT_EQ(diffMap.size(), 2);
  EXPECT_EQ(diffMap["C"], "D");
  EXPECT_EQ(diffMap["D"], "A");
}

TEST(UtilsTest, MapDiff2) {
  unordered_map<string, string> olderMap({{"A", "B"}, {"B", "C"}, {"C", "A"}});
  unordered_map<string, string> newerMap({
      {"A", "C"},
      {"C", "D"},
      {"D", "A"},
  });
  unordered_map<string, string> diffMap;

  bool hasDiff = MemberUtils::diffMap(olderMap, newerMap, diffMap);

  EXPECT_TRUE(hasDiff);
  EXPECT_EQ(diffMap.size(), 4);
  EXPECT_EQ(diffMap["A"], "C");
  EXPECT_EQ(diffMap["B"], KEY_REMOVED);
  EXPECT_EQ(diffMap["C"], "D");
  EXPECT_EQ(diffMap["D"], "A");
}
