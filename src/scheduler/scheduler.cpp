#include <scheduler/scheduler.h>

scheduler::scheduler(scheduler_interface & impl) : impl_(impl) {}

void scheduler::add_periodic_task(task_fn_t task, float interval) {
  impl_.add_periodic_task(task, interval);
}

void scheduler::add_once_task(task_fn_t task, float dt) {
  impl_.add_once_task(task, dt);
}

time_t scheduler::unix_time() {
  return impl_.unix_time();
}
