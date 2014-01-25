#include <glog/logging.h>
#include "gtest/gtest.h"

#include "basic_test_case.hpp"
#include "query_grammar_test_case.hpp"
#include "mock_scheduler_test_case.hpp"
#include "streams_test_case.hpp"
#include "folds_test_case.hpp"
#include "atom_test_case.hpp"
#include "rules_common_test_case.hpp"
#include "scheduler.h"
#include "mock_scheduler.h"
#include "atom.h"

mock_scheduler mock_sched;
scheduler g_scheduler{mock_sched};

int main(int argc, char **argv)
{
  cds::Initialize();
  int ret;
  {
    cds::gc::HP hpGC;
    atom<bool>::attach_thread();

    ::testing::InitGoogleTest(&argc, argv);
    google::InitGoogleLogging(argv[0]);

    ret = RUN_ALL_TESTS();
  }
  cds::Terminate();

  return ret;
}
