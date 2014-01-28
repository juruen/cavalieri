#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include <scheduler.h>
#include <mock_scheduler.h>
#include <external_mocks.h>
#include <rule_tester_util.h>
#include <rules/rules.h>
#include <mock_index.h>

DEFINE_string(input_events, "", "json string containing input events");

mock_scheduler mock_sched;
scheduler g_scheduler{mock_sched};
external_mocks g_external_mocks{};

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  bool ok;
  std::vector<Event> events = json_to_events(FLAGS_input_events, ok);

  if (!ok) {
    LOG(ERROR) << "couldn't parse input events";
    return -1;
  }

  mock_index idx;
  class index index(idx);
  auto rule_stream = rules(index);

  for (const auto & event: events) {
    mock_sched.process_event_time(event.time());
    rule_stream(event);
  }

  std::cout << results(idx.events(), g_external_mocks.calls()) << "\n";

  return 0;
}
