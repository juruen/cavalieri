#include <glog/logging.h>
#include <chrono>
#include <websocket/worker_pool.h>

namespace {

const size_t k_worker_threads = 4;
const long k_check_filter_ms = 2000;

void process_events(worker_pool::filters_map_t & filters,
                    std::mutex & filter_mutex,
                    worker_pool::event_queue_t & queue)
{

  worker_pool::event_t e;
  auto filter_fns(filters);

  auto t1 = std::chrono::system_clock::now();

  while (true) {

    queue.pop(e);

    if (e.stop) {
      VLOG(3) << "finishing process_events";
      return;
    }

    for (const auto & kv : filter_fns) {
      for (const auto & fn : kv.second) {
        fn (e.event);
      }
    }

    auto t2 = std::chrono::system_clock::now();
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>
                (t2 - t1).count();

    if (dt > k_check_filter_ms) {

      t1 = t2;

      std::lock_guard<std::mutex> lock(filter_mutex);

      filter_fns = filters;

    }

  }

}

}


worker_pool::worker_pool() :
  stop_(false), has_clients_(false), filters_(), event_queue_(), mutex_(),
  worker_threads_(k_worker_threads,
                  [&]() { process_events(filters_, mutex_, event_queue_); })
{

  event_queue_.set_capacity(k_max_ws_queue_size);

}

void worker_pool::add_event(const Event & event) {


  if (stop_ || !has_clients_) {
    return;
  }

  if (!event_queue_.try_push({event, false})) {
    LOG(WARNING) << "ws worker_pool queue is full";
  }

}

void worker_pool::update_filters(const size_t id,
                                 const event_filters_t filters) {

  VLOG(3) << "updating event filters";

  {
    std::lock_guard<std::mutex> lock(mutex_);
    filters_[id] = filters;
    has_clients_ = (!filters.empty());
  }

  VLOG(3) << "has_clients_ " << has_clients_;

}

void worker_pool::stop() {

  VLOG(3) << "stopping ";

  stop_ = true;
  event_queue_.clear();

  for (size_t i = 0; i < k_worker_threads; i++) {
    event_queue_.try_push({{}, true});
  }

  worker_threads_.stop();

}
