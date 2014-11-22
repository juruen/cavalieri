#include <glog/logging.h>
#include <pool/executor_thread_pool.h>

namespace {

const std::string k_exec_pool_service = "executor pool queue size";
const std::string k_exec_pool_desc = "number of pending tasks";
const size_t k_stop_attempts = 50;
const size_t k_stop_interval_check_ms  = 100;
const size_t k_max_queue_size = 1e+7;

}

executor_thread_pool::executor_thread_pool(
    instrumentation::instrumentation & instr,
    const config & conf)
  :
    task_guague_(instr.add_gauge(k_exec_pool_service, k_exec_pool_desc)),
    finished_threads_(conf.executor_pool_size, 0),
    next_thread_(0),
    tasks_(conf.executor_pool_size)
{

  auto run_fn = [=](const int i)
  {
    run_tasks(i);
  };


  for (size_t i = 0; i < conf.executor_pool_size; i++) {
    tasks_[i].set_capacity(k_max_queue_size);
    threads_.push_back(std::move(std::thread(run_fn, i)));
  }

}

void executor_thread_pool::add_task(const task_fn_t & task) {

  task_guague_.incr_fn(1);
  tasks_[next_thread_].push({task, false});

  next_thread_ = (next_thread_ + 1) % tasks_.size();

}

void executor_thread_pool::stop() {

  VLOG(3) << "stopping executor_thread_pool";

  for (size_t i = 0; i < threads_.size(); i++) {
    tasks_[i].clear();
    tasks_[i].push({{}, true});
  }

  for (size_t attempts = k_stop_attempts; attempts > 0; attempts--) {

    size_t stopped = 0 ;

    for (const auto & t: finished_threads_) {
      if (t) stopped++;
    }

    if (stopped == threads_.size()) {
      break;
    }

    VLOG(3) << "Waiting for " << threads_.size() - stopped << " threads";

    std::this_thread::sleep_for(
        std::chrono::milliseconds(k_stop_interval_check_ms));
  }

  for (size_t i = 0; i < threads_.size(); i++) {
    threads_[i].join();
  }

}

void executor_thread_pool::run_tasks(const int i) {

  VLOG(3) << "starting executor thread";

  task_t task;

  while (true) {

    tasks_[i].pop(task);

    if (task.stop) {
      break;
    }

    task.fn();

    task_guague_.decr_fn(1);

  }

  finished_threads_[i] = 1;

  VLOG(3) << "run_tasks()--";

}
