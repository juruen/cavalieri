#include <scheduler/scheduler.h>

scheduler::scheduler(std::shared_ptr<scheduler_interface> impl) : impl_(impl) {}

void scheduler::add_periodic_task(task_fn_t task, float interval) {
  impl_->add_periodic_task(task, interval);
}

void scheduler::add_once_task(task_fn_t task, float dt) {
  impl_->add_once_task(task, dt);
}

time_t scheduler::unix_time() {
  return impl_->unix_time();
}

void scheduler::set_time(const time_t t) {
  return impl_->set_time(t);
}

void scheduler::clear() {
  return impl_->clear();
}
