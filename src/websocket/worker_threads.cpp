#include <glog/logging.h>
#include <websocket/worker_threads.h>
#include <atom/atom.h>

namespace {

const size_t k_stop_attempts = 50;
const size_t k_stop_interval_check_ms  = 100;

}

worker_threads::worker_threads(const size_t threads, task_t task)
  :
    task_(task),
    finished_threads_(threads, 0)
{

  auto run_fn = [=](const int i)
  {
    atom_attach_thread();
    run_tasks(i);
    atom_detach_thread();
  };


  for (size_t i = 0; i < threads; i++) {
    threads_.push_back(std::move(std::thread(run_fn, i)));
  }

}

void worker_threads::stop() {

  VLOG(3) << "stopping worker_threads";

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

void worker_threads::run_tasks(const int i) {

  VLOG(3) << "starting worker thread " << i;

  task_();

  finished_threads_[i] = 1;

  VLOG(3) << "stopping worker thread " << i;

}
