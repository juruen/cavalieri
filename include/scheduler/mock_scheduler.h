#ifndef CAVALIERI_SCHEDULER_MOCK_SCHEDULER_H
#define CAVALIERI_SCHEDULER_MOCK_SCHEDULER_H

#include <queue>
#include <vector>
#include <scheduler/scheduler.h>

typedef std::tuple<time_t, time_t, task_fn_t> queue_element_t;

class mock_scheduler_queue_cmp
{
public:
  bool operator() (const queue_element_t & lhs, const queue_element_t & rhs) const
  {
    return (std::get<0>(lhs) > std::get<0>(rhs));
  }
};

class mock_scheduler : public scheduler_interface {
public:
  mock_scheduler();
  remove_task_future_t add_periodic_task(task_fn_t task,
                                         float interval) override;
  remove_task_future_t add_once_task(task_fn_t task, float dt) override;
  time_t unix_time()  override;
  void set_time(const time_t t) override;
  void clear() override;

private:
  void set_forward_time(time_t time);

private:
  time_t unix_time_;
  std::priority_queue<
      queue_element_t,
      std::vector<queue_element_t>,
      mock_scheduler_queue_cmp
    > tasks_;
};

#endif
