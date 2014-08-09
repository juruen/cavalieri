#include <algorithm>
#include <glog/logging.h>
#include <util/util.h>
#include <pool/async_thread_pool.h>
#include <atom/atom.h>

namespace {
  const size_t stop_attempts = 20;
  const size_t stop_interval_check_ms  = 50;
}

using namespace std::placeholders;

async_thread_pool::async_thread_pool(size_t thread_num) :
  stop_(false),
  thread_num_(thread_num),
  next_thread_(0),
  finished_threads_(thread_num, false),
  async_events_(make_async_events(thread_num,
                                  std::bind(&async_thread_pool::async_callback,
                                            this, _1)))
{
  if (thread_num < 1) {
    LOG(FATAL) << "Thread number must be greater than 0";
  }
}

async_thread_pool::async_thread_pool(
    size_t thread_num,
    const float interval,
    timer_cb_fn_t timer_cb_fn
) :
  stop_(false),
  thread_num_(thread_num),
  next_thread_(0),
  finished_threads_(thread_num, false),
  async_events_(make_async_events(thread_num,
                                  std::bind(&async_thread_pool::async_callback,
                                            this, _1),
                                  interval,
                                  timer_cb_fn))
{
  if (thread_num < 1) {
    LOG(FATAL) << "Thread number must be greater than 0";
  }
}

void async_thread_pool::set_run_hook(hook_fn_t hook) {
  run_hook_fn_ = hook;
}

void async_thread_pool::set_async_hook(hook_fn_t hook) {
  async_hook_fn_ = hook;
}

void async_thread_pool::start_threads() {
  auto run_fn = [=](size_t i)
  {
    atom_attach_thread();
    this->run(i);
    atom_detach_thread();
  };

  for (size_t i = 0; i < thread_num_; i++) {
    threads_.push_back(std::move(std::thread(run_fn, i)));
  }
}

void async_thread_pool::async_callback(async_loop & loop) {
  size_t loop_id = loop.id();
  VLOG(3) << "async_callback loop_id: " << loop_id;
  if (!stop_) {
      async_hook_fn_(loop);
  } else {
    loop.stop();
  }
}

void async_thread_pool::run(const size_t loop_id) {
  VLOG(3) << "run() thread id: " << loop_id;

  if (run_hook_fn_) {
    run_hook_fn_(async_events_->loop(loop_id));
  }

  async_events_->start_loop(loop_id);

  mutex_.lock();
  finished_threads_[loop_id] = true;
  mutex_.unlock();
  VLOG(3) << "run() thread id: " << loop_id << " finished";
}


void async_thread_pool::stop_threads() {
  if (stop_) {
    VLOG(3) << "stop_threads(): already stopped";
    return;
  }

  stop_ = true;
  async_events_->stop_all_loops();

  for (size_t attempts = stop_attempts; attempts > 0; attempts--) {

    size_t stopped = 0 ;

    mutex_.lock();
    for (const auto & t: finished_threads_) {
      if (t) stopped++;
    }
    mutex_.unlock();

    if (stopped == thread_num_) {
      break;
    }

    VLOG(3) << "Waiting for " << thread_num_- stopped << " threads";

    std::this_thread::sleep_for(
        std::chrono::milliseconds(stop_interval_check_ms));
  }

  for (size_t i = 0; i < thread_num_; i++) {
      threads_[i].join();
  }

}

void async_thread_pool::signal_thread(size_t loop_id) {
  VLOG(3) << "signal_thread() loop_id: " << loop_id;

  if (loop_id >= thread_num_) {
    LOG(FATAL) << "Invalid loop_id";
    return;
  }

  async_events_->signal_loop(loop_id);
}

size_t async_thread_pool::next_thread() {
  next_thread_ = (next_thread_ + 1) % thread_num_;
  return next_thread_;
}

async_loop & async_thread_pool::loop(const size_t id) {
  return async_events_->loop(id);
}

async_thread_pool::~async_thread_pool() {
  VLOG(3) << "~async_thread_pool()";
  stop_threads();
}
