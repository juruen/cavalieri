#include <glog/logging.h>
#include "gtest/gtest.h"

#include "basic_test_case.hpp"
#include "query_grammar_test_case.hpp"
#include "mock_scheduler_test_case.hpp"
#include "streams_test_case.hpp"
#include "scheduler.h"
#include "mock_scheduler.h"

mock_scheduler mock_sched;
scheduler g_scheduler{mock_sched};

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);

  return RUN_ALL_TESTS();
}
