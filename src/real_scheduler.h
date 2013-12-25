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
  time_t unix_time();

private:
  void timer_callback(ev::timer & timer, int revents);

private:
  std::unordered_map<std::shared_ptr<ev::timer>, task_fn_t> timer_to_task_;
};

#endif

