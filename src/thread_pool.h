#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <ev++.h>
#include <unordered_map>

typedef std::function<void(size_t, ev::dynamic_loop&)> hook_fn_t;

class thread_pool {
  public:
     thread_pool( size_t thread_num);
     void set_run_hook(hook_fn_t);
     void set_async_hook(hook_fn_t);
     void start_threads();
     void stop_threads();
     void signal_thread(size_t tid);
     size_t next_thread();
     size_t ev_to_tid(ev::io & io);
     size_t ev_to_tid(ev::async & async);
     size_t ev_to_tid(ev::timer & timer);
     virtual ~thread_pool();

  protected:
    virtual void run(const size_t thread_id);
    void async_callback(ev::async & async, int revents);

  protected:
    bool stop_;
    size_t thread_num_;
    size_t next_thread_;
    std::unordered_map<uintptr_t, int> loop_to_thread_id_;
    std::vector<std::thread> threads_;
    std::vector<ev::dynamic_loop> loops_;
    std::vector<ev::async> async_watchers_;
    std::vector<bool> finished_threads_;
    hook_fn_t run_hook_fn_;
    hook_fn_t async_hook_fn_;
};
#endif
