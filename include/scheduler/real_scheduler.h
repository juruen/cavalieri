#ifndef SCHEDULER_REAL_SCHEDULER_H
#define SCHEDULER_REAL_SCHEDULER_H

#include <unordered_map>
#include <vector>
#include <memory>
#include <ev++.h>
#include <scheduler/scheduler.h>
#include <async/async_loop.h>

class real_scheduler : public scheduler_interface {
public:
  real_scheduler(main_async_loop_interface & main_loop);
  void add_periodic_task(task_fn_t task, float interval);
  void add_once_task(task_fn_t task, float dt);
  time_t unix_time();
  void set_time(const time_t t);
  void clear();

private:
  void timer_callback(ev::timer & timer, int revents);

private:
  typedef std::pair<task_fn_t, bool> sched_task_t;

  main_async_loop_interface & main_loop_;


};

#endif
