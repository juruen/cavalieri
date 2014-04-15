#include <glog/logging.h>
#include <scheduler/real_scheduler.h>
#include <async/async_loop.h>
#include <util.h>

namespace {
  const bool periodic_task = true;
  const bool once_task = false;
}

void real_scheduler::add_periodic_task(task_fn_t task, float interval) {
}

void real_scheduler::add_once_task(task_fn_t, float) {
}

time_t real_scheduler::unix_time() {
  return time(0);
}

void real_scheduler::timer_callback(ev::timer &, int) {
}

real_scheduler real_sched;
scheduler g_scheduler{real_sched};
