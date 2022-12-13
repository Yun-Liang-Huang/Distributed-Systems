cc_library(
  name = "lib",
  srcs = glob(["src/*.cpp"]),
  hdrs = glob(["src/*.h"]),
)

cc_test(
  name = "query_test",
  size = "small",
  srcs = glob(["test/query_test.cc"]),
  data = glob(["logs/**"]),
  deps = ["@com_google_googletest//:gtest_main", ":lib"],
)

cc_test(
  name = "utils_test",
  size = "small",
  srcs = glob(["test/utils_test.cc"]),
  deps = ["@com_google_googletest//:gtest_main", ":lib"],
)

cc_test(
  name = "member_test",
  size = "small",
  srcs = glob(["test/member_test.cc"]),
  deps = ["@com_google_googletest//:gtest_main", ":lib"],
)