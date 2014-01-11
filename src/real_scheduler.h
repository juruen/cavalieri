#ifndef REAL_SCHEDULER_H
#define REAL_SCHEDULER_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <ev++.h>
#include <scheduler.h>

class real_scheduler : public scheduler_interface {
public:
  void add_periodic_task(task_fn_t task, float interval);
  void add_once_task(task_fn_t task, float dt);
  time_t unix_time();

private:
  void timer_callback(ev::timer & timer, int revents);

private:
  typedef std::pair<task_fn_t, bool> sched_task_t;
  std::unordered_map<std::shared_ptr<ev::timer>, sched_task_t> timer_to_task_;
};

#endif
