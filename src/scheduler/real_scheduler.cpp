#include <glog/logging.h>
#include <scheduler/real_scheduler.h>
#include <async/async_loop.h>
#include <util/util.h>

real_scheduler::real_scheduler(main_async_loop_interface & main_loop)
  : main_loop_(main_loop)
{
}

void real_scheduler::add_periodic_task(task_fn_t task, float interval) {
  main_loop_.add_periodic_task(task, interval);
}

void real_scheduler::add_once_task(task_fn_t, float) {
}

time_t real_scheduler::unix_time() {
  return time(0);
}

void real_scheduler::timer_callback(ev::timer &, int) {
}

void real_scheduler::set_time(const time_t) { }

void real_scheduler::clear() { }
