#include <mock_scheduler.h>

mock_scheduler::mock_scheduler()
:
  unix_time_(0),
  tasks_(mock_scheduler_queue_cmp())
{
}

void mock_scheduler::add_periodic_task(task_fn_t task, float interval) {
  auto p = static_cast<time_t>(interval);
  tasks_.push(std::make_tuple(unix_time_ + p, p, task));
}

time_t mock_scheduler::unix_time() {
  return unix_time_;
}

void mock_scheduler::process_event_time(time_t event_time) {
  while (!tasks_.empty()) {
    auto lowest = std::get<0>(tasks_.top());
    if (lowest > event_time) {
      break;
    }
    auto interval = std::get<1>(tasks_.top());
    auto fn = std::get<2>(tasks_.top());
    fn();
    tasks_.push(std::make_tuple(unix_time_ + interval, interval, fn));
  }
}
