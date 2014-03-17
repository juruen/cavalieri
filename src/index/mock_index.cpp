#include <index/mock_index.h>
#include <scheduler/scheduler.h>

void mock_index::add_event(const Event & e) {
  events_.push_back({g_scheduler.unix_time(), e});
}

std::vector<mock_index_events_t> mock_index::events() const {
  return events_;
}
