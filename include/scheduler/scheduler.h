#ifndef CAVALIERI_SCHEDULER_SCHEDULER_H
#define CAVALIERI_SCHEDULER_SCHEDULER_H

#include <functional>
#include <memory>
#include <time.h>
#include <async/async_loop.h>

typedef std::function<void()> task_fn_t;

class scheduler_interface {
public:
  virtual void add_periodic_task(task_fn_t task, float interval) = 0;
  virtual void add_once_task(task_fn_t task, float dt) = 0;
  virtual time_t unix_time() = 0;
  virtual void set_time(const time_t t) = 0;
  virtual void clear() = 0;
  virtual ~scheduler_interface() {};
};

#endif
