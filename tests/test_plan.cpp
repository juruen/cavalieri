#include <glog/logging.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "basic_test_case.hpp"
#include "query_grammar_test_case.hpp"
#include "mock_scheduler_test_case.hpp"
#include "streams_test_case.hpp"
#include "folds_test_case.hpp"
#include "atom_test_case.hpp"
#include "rules_common_test_case.hpp"
#include "tcp_connection_test_case.hpp"
#include "ws_connection_test_case.hpp"
#include "riemann_tcp_connection_test_case.hpp"
#include "pubsub_test_case.hpp"
#include "index_test_case.hpp"
#include <scheduler/scheduler.h>
#include <scheduler/mock_scheduler.h>
#include "os_functions.h"
#include "mock_os_functions.h"
#include "atom/atom.h"

mock_scheduler mock_sched;
scheduler g_scheduler{mock_sched};

mock_os_functions mock_os;
os_functions g_os_functions(mock_os);

int main(int argc, char **argv)
{
  atom_initialize();
  int ret;
  {
    ATOM_GC;
    atom_attach_thread();

    ::testing::InitGoogleTest(&argc, argv);
    google::InitGoogleLogging(argv[0]);

    ::testing::InitGoogleMock(&argc, argv);

    ret = RUN_ALL_TESTS();
  }
  atom_terminate();

  return ret;
}
