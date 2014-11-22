#ifndef CAVALIERI_INDEX_REAL_INDEX_H
#define CAVALIERI_INDEX_REAL_INDEX_H

#include <common/event.h>
#include <atomic>
#include <mutex>
#include <pub_sub/pub_sub.h>
#include <scheduler/scheduler.h>
#include <instrumentation/instrumentation.h>
#include <index/index.h>

class real_index : public index_interface {
public:
  real_index(pub_sub & pubsub, push_event_fn_t push_event,
             const int64_t expire_interval,
             scheduler_interface &  sched,
             instrumentation::instrumentation & instr,
             spwan_thread_fn_t spwan_thread_fn);
  std::vector<Event> query_index(const match_fn_t, const size_t max_matches);
  void add_event(const Event& e);
  ~real_index();

private:
  void timer_cb();
  void expire_events();

private:
  using real_index_t = std::unordered_map<std::string, Event>;

private:
  pub_sub & pubsub_;
  instrumentation::update_gauge_t pre_gauge_;
  instrumentation::update_gauge_t post_gauge_;
  push_event_fn_t push_event_fn_;
  std::atomic<bool> expiring_;
  spwan_thread_fn_t spwan_thread_fn_;
  scheduler_interface & sched_;
  std::vector<real_index_t> indexes_;
  std::vector<std::mutex> mutexes_;

};

#endif
