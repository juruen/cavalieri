#ifndef CAVALIERI_WEBSOCKET_WORKER_POOL_H
#define CAVALIERI_WEBSOCKET_WORKER_POOL_H

#include <mutex>
#include <unordered_map>
#include <tbb/concurrent_queue.h>
#include <websocket/common.h>
#include <websocket/worker_threads.h>

class worker_pool {
public:
  worker_pool();
  worker_pool(worker_pool const&) = delete;
  void add_event(const Event & event);
  void update_filters(const size_t id, const event_filters_t);
  void stop();

public:
  using event_t = struct {
    Event event;
    bool stop;
  };

  using event_queue_t = tbb::concurrent_bounded_queue<event_t>;

  using filters_map_t = std::unordered_map<size_t, event_filters_t>;

private:

  bool stop_, has_clients_;
  filters_map_t filters_;
  event_queue_t event_queue_;
  std::mutex mutex_;
  worker_threads worker_threads_;

};

#endif
