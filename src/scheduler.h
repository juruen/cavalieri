#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <functional>
#include <time.h>

typedef std::function<void()> task_fn_t;

class scheduler_interface {
public:
  virtual void add_periodic_task(task_fn_t task, float interval) = 0;
  virtual time_t unix_time() = 0;
};

class scheduler {
public:
  scheduler(scheduler_interface & impl);
  void add_periodic_task(std::function<void()> task, float interval);
  time_t unix_time();

private:
  scheduler_interface & impl_;
};

extern scheduler g_scheduler;

#endif
