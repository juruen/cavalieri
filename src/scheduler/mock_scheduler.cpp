#include <scheduler/mock_scheduler.h>

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

void mock_scheduler::add_once_task(task_fn_t task, float dt) {
  auto p = static_cast<time_t>(dt);
  tasks_.push(std::make_tuple(unix_time_ + p, 0, task));
}

time_t mock_scheduler::unix_time() {
  return unix_time_;
}

void mock_scheduler::set_time(const time_t event_time) {
  while (!tasks_.empty()) {
    auto lowest = std::get<0>(tasks_.top());
    if (lowest > event_time) {
      break;
    }
    set_forward_time(lowest);
    auto interval = std::get<1>(tasks_.top());
    auto fn = std::get<2>(tasks_.top());
    tasks_.pop();
    fn();
    if (interval) {
      tasks_.push(std::make_tuple(unix_time_ + interval, interval, fn));
    }
  }
  set_forward_time(event_time);
}

void mock_scheduler::set_forward_time(time_t time) {
  if (time > unix_time_) {
    unix_time_ = time;
  }
}

void mock_scheduler::clear() {
  while (!tasks_.empty()) {
    tasks_.pop();
  }
  unix_time_ = 0;
}


