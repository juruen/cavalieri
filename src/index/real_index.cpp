#include <thread>
#include <queue>
#include <glog/logging.h>
#include <util.h>
#include <atom/atom.h>
#include <core/core.h>
#include <index/real_index.h>

namespace {

const std::string k_default_index = "index";
const std::string k_preexpire_service = "cavalieri index size pre-expire";
const std::string k_preexpire_desc = "number of events before removing expired";
const std::string k_postexpire_service = "cavalieri index size post-expire";
const std::string k_postexpire_desc = "number of events after removing expired";
const size_t k_stop_attempts = 120;
const size_t k_stop_interval_check_ms = 500;
const size_t k_max_queue_events = 10000000;
const size_t k_queue_id = 0;
const size_t k_expire_id = 1;

std::string key(const Event& e) {
  return e.host() + "-" + e.service();
}

}

using namespace std::placeholders;

real_index::real_index(pub_sub & pubsub, push_event_fn_t push_event,
                       const int64_t expire_interval,
                       scheduler_interface &  sched,
                       instrumentation & instr,
                       spwan_thread_fn_t spwan_thread_fn)
:
  pubsub_(pubsub),
  instrumentation_(instr),
  instr_ids_({instr.add_gauge(k_preexpire_service, k_preexpire_desc),
              instr.add_gauge(k_postexpire_service, k_postexpire_desc)}),
  push_event_fn_(push_event),
  expiring_(false),
  spwan_thread_fn_(spwan_thread_fn),
  sched_(sched),
  events_(),
  pool_(2, static_cast<float>(expire_interval),
        std::bind(&real_index::real_index::expire_events, this, _1)),
  stop_(false)
{

  events_.set_capacity(k_max_queue_events);

  pubsub_.add_publisher(k_default_index,
                        std::bind(&real_index::all_events, this));

  pool_.set_run_hook(std::bind(&real_index::dequeue_events, this, _1));

  pool_.start_threads();
}

std::vector<std::shared_ptr<Event>> real_index::all_events() {

  mutex_.lock();
  auto idx_cpy(index_map_);
  mutex_.unlock();

  std::vector<std::shared_ptr<Event>> events;

  for (const auto & kv: idx_cpy) {
    events.push_back(kv.second);
  }

  return events;
}

void real_index::add_event(const Event& e) {

  VLOG(3) << "add_event()";

  if (!events_.try_push(std::make_shared<Event>(e))) {
    LOG(ERROR) << "index queue is full";
  }

}

void real_index::dequeue_events(async_loop & loop) {

  if (loop.id() != k_queue_id) {
    return;
  }

  VLOG(3) << "start dequeue_events() from index";

  std::shared_ptr<Event> event;

  while (true) {

    events_.pop(event);

    if (!event) {

      VLOG(3) << "dequeue_events() stop";
      return;

    }

    const std::string ev_key(key(*event));

    {
      std::lock_guard<std::mutex> lock(mutex_);
      index_map_[ev_key] = event;
    }

    pubsub_.publish(k_default_index, *event);

  }

}

void real_index::stop() {
  VLOG(3) << "stop()";

  stop_ = true;

  events_.clear();
  events_.try_push(std::shared_ptr<Event>());

  pool_.stop_threads();
}

void real_index::expire_events(async_loop & loop) {

  if (loop.id() != k_expire_id) {
    return;
  }

  VLOG(3) << "expire_fn()++";

  instrumentation_.update_gauge(instr_ids_.first, index_map_.size());

  std::vector<std::shared_ptr<Event>> expired_events;

  int64_t now = static_cast<int64_t>(sched_.unix_time());

  {

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = index_map_.begin();
    while (it != index_map_.end()) {

      const auto & event(it->second);

      auto expire = event->time() + static_cast<int64_t>(event->ttl());
      if (expire < now) {
        expired_events.push_back(event);
        index_map_.erase(it++);
      } else {
        ++it;
      }

    }

  }

  instrumentation_.update_gauge(instr_ids_.second, index_map_.size());

  VLOG(3) << "expire process took "
          << static_cast<int64_t>(sched_.unix_time()) - now << " seconds";

  for (auto & event : expired_events) {
    Event e(*event);
    e.set_state("expired");
    pubsub_.publish(k_default_index, e);
    push_event_fn_(e);
  }

  VLOG(3) << "expire_fn()--";

}

real_index::~real_index() {

  VLOG(3) << "~real_index()++";

  if (!stop_) {
    stop();
  }

}
