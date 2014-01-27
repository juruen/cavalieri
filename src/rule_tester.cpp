#include <gflags/gflags.h>
#include "scheduler.h"
#include "mock_scheduler.h"

DEFINE_string(input_events, "", "json string containing input events");

mock_scheduler mock_sched;
scheduler g_scheduler{mock_sched};

#include <iostream>
int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  return 0;
}
