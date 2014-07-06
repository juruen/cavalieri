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
const size_t k_preexpire_guage_id = 0;
const std::string k_postexpire_service = "cavalieri index size post-expire";
const std::string k_postexpire_desc = "number of events after removing expired";
const size_t k_postexpire_gauge_id = 1;
const std::string k_queue_service = "cavalieri index queue";
const std::string k_queue_desc = "number of events in index queue";
const size_t k_queue_gauge_id = 2;

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
  push_event_fn_(push_event),
  expiring_(false),
  spwan_thread_fn_(spwan_thread_fn),
  sched_(sched),
  events_(),
  pool_(2, static_cast<float>(expire_interval),
        std::bind(&real_index::real_index::run, this, _1)),
  stop_(false)
{

  events_.set_capacity(k_max_queue_events);

  pubsub_.add_publisher(k_default_index,
                        std::bind(&real_index::all_events, this));

  instrs_ids_.push_back(instr.add_gauge(k_preexpire_service,
                                        k_preexpire_desc));

  instrs_ids_.push_back(instr.add_gauge(k_postexpire_service,
                                        k_postexpire_desc));

  instrs_ids_.push_back(instr.add_gauge(k_queue_service,
                                        k_queue_desc));


  pool_.start_threads();
}

std::vector<std::shared_ptr<Event>> real_index::all_events() {

  std::vector<std::shared_ptr<Event>> events;

  for (const auto & kv: index_map_) {
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

void real_index::run(async_loop & loop) {
    if (loop.id() == k_queue_id) {
      dequeue_events();
    } else {
      expire_events();
    }
}

void real_index::dequeue_events() {

  VLOG(3) << "start dequeue_events() from index";

  std::shared_ptr<Event> event;

  while (true) {

    events_.pop(event);

    if (!event) {

      VLOG(3) << "dequeue_events() stop";
      return;

    }

    const std::string ev_key(key(*event));

    index_map_.erase(ev_key);
    index_map_.insert({ev_key,  event});

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

void real_index::expire_events() {

  VLOG(3) << "expire_fn()++";

  instrumentation_.update_gauge(instrs_ids_[k_preexpire_guage_id],
                                index_map_.size());

  auto queue_size = events_.size();
  instrumentation_.update_gauge(instrs_ids_[k_queue_gauge_id],
                                queue_size < 0 ? 0 : queue_size);

  std::vector<std::string> keys_to_remove;
  std::vector<Event> expired_events;

  int64_t now = static_cast<int64_t>(sched_.unix_time());

  for (const auto & pair : index_map_) {

    auto  event = pair.second;

    auto expire = event->time() + static_cast<int64_t>(event->ttl());
    if (expire < now) {
      keys_to_remove.push_back(pair.first);
      expired_events.push_back(*event);
    }

  }

  for (const auto & key : keys_to_remove) {
    index_map_.erase(key);
  }

  for (auto & event : expired_events) {
    event.set_state("expired");
    pubsub_.publish(k_default_index, event);
    push_event_fn_(event);
  }

  instrumentation_.update_gauge(instrs_ids_[k_postexpire_gauge_id],
                                index_map_.size());

  VLOG(3) << "expire process took "
          << static_cast<int64_t>(sched_.unix_time()) - now << " seconds";

  VLOG(3) << "expire_fn()--";

}

real_index::~real_index() {

  VLOG(3) << "~real_index()++";

  if (!stop_) {
    stop();
  }

}
