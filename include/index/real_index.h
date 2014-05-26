#ifndef INDEX_REAL_INDEX_H
#define INDEX_REAL_INDEX_H

#include <proto.pb.h>
#include <atomic>
#include <mutex>
#include <pub_sub/pub_sub.h>
#include <scheduler/scheduler.h>
#include <index/index.h>

class real_index : public index_interface {
public:
  real_index(pub_sub & pubsub, push_event_fn_t push_event,
             const int64_t expire_interval,
             scheduler_interface &  sched,
             spwan_thread_fn_t spwan_thread_fn);
  ~real_index();
  void add_event(const Event& e);

private:
  void timer_cb();
  void expire_events();
  std::vector<std::shared_ptr<Event>> all_events();

private:
  typedef std::unordered_map<std::string, std::shared_ptr<Event>> real_index_t;

  pub_sub& pubsub_;
  push_event_fn_t push_event_fn_;
  std::atomic<bool> expiring_;
  spwan_thread_fn_t spwan_thread_fn_;
  scheduler_interface & sched_;
  real_index_t index_map_;
  std::mutex mutex_;

};

#endif
