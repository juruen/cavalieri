#ifndef CAVALIERI_SCHEDULER_SCHEDULER_H
#define CAVALIERI_SCHEDULER_SCHEDULER_H

#include <functional>
#include <future>
#include <time.h>
#include <async/async_loop.h>

using task_fn_t = std::function<void()>;
using remove_task_fn_t = std::function<void()>;
using remove_task_future_t = std::future<remove_task_fn_t>;
using remove_ns_tasks_future_t = std::future<bool>;

class scheduler_interface {
public:
  virtual remove_task_future_t add_periodic_task(task_fn_t task,
                                                  float interval) = 0;
  virtual remove_task_future_t add_once_task(task_fn_t task, float dt) = 0;
  virtual remove_ns_tasks_future_t remove_ns_tasks(const std::string nm) = 0;
  virtual time_t unix_time() = 0;
  virtual void set_time(const time_t t) = 0;
  virtual void clear() = 0;
  virtual ~scheduler_interface() {};
};

#endif
