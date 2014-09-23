#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include <scheduler/scheduler.h>
#include <scheduler/mock_scheduler.h>
#include <rule_tester_util.h>
#include <util/util.h>
#include <rules_loader.h>
#include <core/mock_core.h>
#include <index/mock_index.h>
#include <external/mock_external.h>

DEFINE_string(input_events, "", "json string containing input events");
DEFINE_string(rules_directory, ".", "directory containing the rules to test");

int main(int argc, char **argv) {


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

  auto m_core = std::make_shared<mock_core>();
  g_core = std::dynamic_pointer_cast<mock_core>(m_core);

  if (!ok) {
    LOG(ERROR) << "couldn't parse input events";
    return -1;
  }

  start_core(argc, argv);

  auto rules = load_rules(FLAGS_rules_directory);

  if (rules.empty()) {
    LOG(ERROR) << "failed to load rules";
    return -1;
  }

  for (const auto & event: events) {
    g_core->sched().set_time(event.time());
    for (const auto & stream : rules) {
      push_event(*stream, event);
    }
  }

  auto & idx = m_core->mock_index_impl();
  std::cout << results(idx.events(), m_core->mock_external_impl().calls())
            << "\n";


  return 0;
}
