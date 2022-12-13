#include <gtest/gtest.h>

#include "../src/member_ping_request.h"
#include "../src/member_utils.h"

using namespace std;

TEST(MemberTest, SerializeDeserializeHashmap) {
  unordered_map<string, string> hashmap1({{"A", "B"}, {"B", "C"}, {"C", "A"}});

  unordered_map<string, string> hashmap2(
      {{"1663717272:fa22-cs425-0410.cs.illinois.edu:8088",
        "1663717278:fa22-cs425-0401.cs.illinois.edu:8088"},
       {"1663717278:fa22-cs425-0401.cs.illinois.edu:8088",
        "1663717252:fa22-cs425-0404.cs.illinois.edu:12345"},
       {"1663717278:fa22-cs425-0404.cs.illinois.edu:12345",
        "1663717252:fa22-cs425-0405.cs.illinois.edu:12344"},
       {"1663717252:fa22-cs425-0405.cs.illinois.edu:12344",
        "1663717272:fa22-cs425-0410.cs.illinois.edu:8088"}});

  unordered_map<string, string> deserialzied_hashmap1;
  unordered_map<string, string> deserialzied_hashmap2;

  string output1 = SocketMessage::serialize_hashmap(hashmap1);
  string output2 = SocketMessage::serialize_hashmap(hashmap2);

  SocketMessage::deserialize_hashmap(output1, deserialzied_hashmap1);
  SocketMessage::deserialize_hashmap(output2, deserialzied_hashmap2);

  EXPECT_FALSE(MemberUtils::diffMap(hashmap1, deserialzied_hashmap1));
  EXPECT_FALSE(MemberUtils::diffMap(hashmap2, deserialzied_hashmap2));
}

TEST(MemberTest, DeserializeHashmap) {
  unordered_map<string, string> hashmap1({{"A", "B"}, {"B", "C"}, {"C", "A"}});

  unordered_map<string, string> hashmap2(
      {{"1663717272:fa22-cs425-0410.cs.illinois.edu:8088",
        "1663717278:fa22-cs425-0401.cs.illinois.edu:8088"},
       {"1663717278:fa22-cs425-0401.cs.illinois.edu:8088",
        "1663717252:fa22-cs425-0404.cs.illinois.edu:12345"},
       {"1663717252:fa22-cs425-0404.cs.illinois.edu:12345",
        "1663717252:fa22-cs425-0405.cs.illinois.edu:12344"},
       {"1663717252:fa22-cs425-0405.cs.illinois.edu:12344",
        "1663717272:fa22-cs425-0410.cs.illinois.edu:8088"}});

  unordered_map<string, string> deserialzied_hashmap1;
  unordered_map<string, string> deserialzied_hashmap2;

  string hashmap_str_1 = "1:A1:B1:B1:C1:C1:A";
  string hashmap_str_2 =
      "47:1663717272:fa22-cs425-0410.cs.illinois.edu:8088"
      "47:1663717278:fa22-cs425-0401.cs.illinois.edu:8088"
      "47:1663717278:fa22-cs425-0401.cs.illinois.edu:8088"
      "48:1663717252:fa22-cs425-0404.cs.illinois.edu:12345"
      "48:1663717252:fa22-cs425-0404.cs.illinois.edu:12345"
      "48:1663717252:fa22-cs425-0405.cs.illinois.edu:12344"
      "48:1663717252:fa22-cs425-0405.cs.illinois.edu:12344"
      "47:1663717272:fa22-cs425-0410.cs.illinois.edu:8088";

  SocketMessage::deserialize_hashmap(hashmap_str_1, deserialzied_hashmap1);
  SocketMessage::deserialize_hashmap(hashmap_str_2, deserialzied_hashmap2);

  EXPECT_FALSE(MemberUtils::diffMap(hashmap1, deserialzied_hashmap1));
  EXPECT_FALSE(MemberUtils::diffMap(hashmap2, deserialzied_hashmap2));
}