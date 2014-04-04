#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include <scheduler/scheduler.h>
#include <scheduler/mock_scheduler.h>
#include <external_mocks.h>
#include <rule_tester_util.h>
#include <util.h>
#include <rules_loader.h>
#include <index/mock_index.h>
#include "atom/atom.h"

DEFINE_string(input_events, "", "json string containing input events");
DEFINE_string(rules_directory, ".", "directory containing the rules to test");

mock_scheduler mock_sched;
scheduler g_scheduler{mock_sched};
external_mocks g_external_mocks{};

int main(int argc, char **argv) {

  atom_initialize();
  {
    ATOM_GC;
    atom_attach_thread();

    int orig_argc = argc;
    char **orig_argv = copy_args(argc, argv);

    FLAGS_logtostderr = true;
    FLAGS_v = 1;

    google::ParseCommandLineFlags(&argc, &argv, true);

    google::InitGoogleLogging(argv[0]);

    ld_environment(orig_argv, FLAGS_rules_directory);

    free_args(orig_argc, orig_argv);

    bool ok;
    std::vector<Event> events = json_to_events(FLAGS_input_events, ok);

    if (!ok) {
      LOG(ERROR) << "couldn't parse input events";
      return -1;
    }

    auto idx = std::make_shared<mock_index>();

    class index index(std::dynamic_pointer_cast<index_interface>(idx));

    auto rules = load_rules(FLAGS_rules_directory);

    if (rules.empty()) {
      LOG(ERROR) << "failed to load rules";
      return -1;
    }

    for (const auto & event: events) {
      mock_sched.process_event_time(event.time());
      for (const auto & stream : rules) {
        push_event(*stream, event);
      }
    }

    std::cout << results(idx->events(), g_external_mocks.calls()) << "\n";
  }
  atom_terminate();

  return 0;
}
