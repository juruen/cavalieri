#include <glog/logging.h>
#include <scheduler/real_scheduler.h>
#include <async/async_loop.h>
#include <util/util.h>

namespace {

const size_t k_scheduler_threads = 2;
const size_t k_max_queue_size = 2000;

}

using namespace std::placeholders;

real_scheduler::real_scheduler() :
  threads_(k_scheduler_threads),
  task_promises_(k_scheduler_threads),
  remove_tasks_(k_scheduler_threads)
{

  VLOG(3) << "real_scheduler()";

  threads_.set_async_hook(std::bind(&real_scheduler::async_callback, this, _1));

  for (size_t i = 0; i < k_scheduler_threads; i++) {
    task_promises_[i].set_capacity(k_max_queue_size);
    remove_tasks_[i].set_capacity(k_max_queue_size);
  }

  threads_.start_threads();
}

remove_task_future_t real_scheduler::add_periodic_task(task_fn_t task,
                                                        float interval)
{

  return add_task(task, interval, false);

}

remove_task_future_t real_scheduler::add_once_task(task_fn_t task,
                                                    float interval)
{

  return add_task(task, interval, true);

}

std::future<remove_task_fn_t> real_scheduler::add_task(
  const task_fn_t task,
  const float interval,
  const bool once)
{
  task_promise_t promise{std::make_shared<std::promise<remove_task_fn_t>>(),
                         task,
                         interval,
                         once};

  auto thread_id = threads_.next_thread();

  if (task_promises_[thread_id].try_push(promise)) {
    threads_.signal_thread(thread_id);
  } else {
    LOG(WARNING) << "scheduler task queue is full";
  }

  return promise.promise->get_future();

}

time_t real_scheduler::unix_time() {
  return time(0);
}

void real_scheduler::async_callback(async_loop & loop) {

  auto id = loop.id();

  task_promise_t tp;

  while (task_promises_[id].try_pop(tp)) {

    timer_id_t timer_id;

    auto timer_task = [=](const size_t) { tp.task(); };

    if (tp.once) {
      timer_id = loop.add_once_task(timer_task, tp.interval);
    } else {
      timer_id = loop.add_periodic_task(timer_task, tp.interval);
    }

    tp.promise->set_value([=]() { threads_.loop(id).remove_task(timer_id); });

  }

}

void real_scheduler::set_time(const time_t) { }

void real_scheduler::clear() { }

void real_scheduler::stop() {

  VLOG(3) << "stop scheduler";
  threads_.stop_threads();

}
