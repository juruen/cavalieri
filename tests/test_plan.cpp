#include <glog/logging.h>
#include "gtest/gtest.h"

#include "basic_test_case.hpp"
#include "query_grammar_test_case.hpp"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);

  return RUN_ALL_TESTS();
}
