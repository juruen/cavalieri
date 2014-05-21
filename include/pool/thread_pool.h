#ifndef POOL_THREAD_POOL_H
#define POOL_THREAD_POOL_H

#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include <async/async_loop.h>

typedef std::function<void(async_loop&)> hook_fn_t;

class thread_pool {
  public:
     thread_pool(size_t thread_num);
     thread_pool(size_t thread_num,
                 const float interval,
                 timer_cb_fn_t timer_cb
      );
     void set_run_hook(hook_fn_t);
     void set_async_hook(hook_fn_t);
     void start_threads();
     void stop_threads();
     void signal_thread(size_t tid);
     size_t next_thread();
     async_loop & loop(const size_t id);
     virtual ~thread_pool();

  private:
    virtual void run(const size_t thread_id);
    void async_callback(async_loop&);

  protected:
    bool stop_;
    size_t thread_num_;
    size_t next_thread_;
    std::vector<std::thread> threads_;
    std::vector<bool> finished_threads_;
    std::unique_ptr<async_events_interface> async_events_;
    hook_fn_t run_hook_fn_;
    hook_fn_t async_hook_fn_;
    std::mutex mutex_;
};

#endif
