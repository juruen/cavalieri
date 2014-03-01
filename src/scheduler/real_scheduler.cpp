#include <glog/logging.h>
#include <scheduler/real_scheduler.h>
#include <util.h>

namespace {
  const bool periodic_task = true;
  const bool once_task = false;
}

void real_scheduler::add_periodic_task(task_fn_t task, float interval) {
  auto t = std::make_shared<ev::timer>();
  t->set<real_scheduler, &real_scheduler::timer_callback>(this);
  t->start(0, interval);
  timer_to_task_.insert({t, {task, periodic_task}});
}

void real_scheduler::add_once_task(task_fn_t task, float dt) {
  auto t = std::make_shared<ev::timer>();
  t->set<real_scheduler, &real_scheduler::timer_callback>(this);
  t->start(dt);
  timer_to_task_.insert({t, {task, once_task}});
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

  auto fn = it->second.first;
  auto periodic = it->second.second;

  fn();

  if (!periodic) {
    timer_to_task_.erase(it);
  }
}

real_scheduler real_sched;
scheduler g_scheduler{real_sched};
