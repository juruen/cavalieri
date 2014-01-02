#include <algorithm>
#include <glog/logging.h>
#include <util.h>
#include <thread_pool.h>
#include <atom.h>

namespace {
  uintptr_t to_uintptr(const ev::loop_ref & loop) {
    return (uintptr_t)(void*)loop.raw_loop;
  }
  const size_t stop_attempts = 20;
  const size_t stop_interval_check_ms  = 50;
}

thread_pool::thread_pool( size_t thread_num) :
  stop_(false),
  thread_num_(thread_num),
  next_thread_(0),
  loops_(thread_num),
  async_watchers_(thread_num),
  finished_threads_(thread_num, false)
{
  if (thread_num < 1) {
    LOG(FATAL) << "Thread number must be greater than 0";
  }

  VLOG(2) << "Creating a pool thread of size: " << thread_num;

  for (size_t i = 0; i < thread_num; i++)  {
    loop_to_thread_id_.insert({to_uintptr(loops_[i]), i});
    async_watchers_[i].set(loops_[i]);
    async_watchers_[i].set<thread_pool, &thread_pool::async_callback>(this);
  }
}

void thread_pool::set_run_hook(hook_fn_t hook) {
  run_hook_fn_ = hook;
}

void thread_pool::set_async_hook(hook_fn_t hook) {
  async_hook_fn_ = hook;
}

void thread_pool::start_threads() {
  auto run_fn = [=](size_t i)
  {
    atom<bool>::attach_thread();
    this->run(i);
    atom<bool>::detach_thread();
  };

  for (size_t i = 0; i < thread_num_; i++) {
    threads_.push_back(std::move(std::thread(run_fn, i)));
  }
}

void thread_pool::async_callback(ev::async & async, int revents) {
  UNUSED_VAR(revents);
  size_t tid = ev_to_tid(async);
  VLOG(3) << "async_callback tid: " << tid;
  if (!stop_) {
    if (async_hook_fn_) {
      async_hook_fn_(tid, loops_[tid]);
    } else {
      VLOG(3) << "aync_hook_fn_ not set";
    }
  } else {
    loops_[tid].unloop();
  }
}

void thread_pool::run(const size_t thread_id) {
  VLOG(3) << "run() thread id: " << thread_id;

  if (run_hook_fn_) {
    run_hook_fn_(thread_id, loops_[thread_id]);
  }

  async_watchers_[thread_id].start();
  loops_[thread_id].run();
  finished_threads_[thread_id] = true;
  VLOG(3) << "run() thread id: " << thread_id << " finished";
}


void thread_pool::stop_threads() {
  if (stop_) {
    VLOG(3) << "stop_threads(): already stopped";
    return;
  }

  stop_ = true;

  for (size_t i = 0; i < thread_num_; i++) {
    signal_thread(i);
  }

  for (size_t attempts = stop_attempts; attempts > 0; attempts-- ) {
    size_t stopped = std::count(
                                 begin(finished_threads_),
                                 end(finished_threads_), true);

    if (stopped == thread_num_) {
      break;
    }

    VLOG(3) << "Waiting for " << thread_num_- stopped << " threads";
    std::this_thread::sleep_for(
        std::chrono::milliseconds(stop_interval_check_ms));
  }

  for (size_t i = 0; i < thread_num_; i++) {
    if (finished_threads_[i]) {
      threads_[i].join();
    }
  }

}

void thread_pool::signal_thread(size_t tid) {
  VLOG(3) << "signal_thread() tid: " << tid;

  if (tid >= thread_num_) {
    LOG(FATAL) << "Invalid tid";
    return;
  }

  async_watchers_[tid].send();
}

size_t thread_pool::ev_to_tid(ev::io & io) {
  return loop_to_thread_id_[to_uintptr(io.loop)];
}

size_t thread_pool::ev_to_tid(ev::async & async) {
  return loop_to_thread_id_[to_uintptr(async.loop)];
}

size_t thread_pool::ev_to_tid(ev::timer & timer) {
  return loop_to_thread_id_[to_uintptr(timer.loop)];
}

size_t thread_pool::next_thread() {
  next_thread_ = (next_thread_ + 1) % thread_num_;
  return next_thread_;
}

thread_pool::~thread_pool() {
  VLOG(3) << "~thread_pool()";
  stop_threads();
}
