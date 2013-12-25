#include <glog/logging.h>
#include <real_scheduler.h>
#include <util.h>

void real_scheduler::add_periodic_task(task_fn_t task, float interval) {
  auto t = std::make_shared<ev::timer>();
  t->set<real_scheduler, &real_scheduler::timer_callback>(this);
  t->start(0, interval);
  timer_to_task_.insert({t, task});
}

time_t real_scheduler::unix_time() {
  return time(0);
}

void real_scheduler::timer_callback(ev::timer & timer, int revents) {
  UNUSED_VAR(revents);

  auto it = timer_to_task_.find({&timer, [](ev::timer*){}});
  if (it == timer_to_task_.end()) {
    LOG(FATAL) << "timer_callback can't find timer ref";
  }

  it->second();
}

real_scheduler real_sched;
scheduler g_scheduler{real_sched};
