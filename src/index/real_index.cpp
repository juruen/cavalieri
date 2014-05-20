#include <thread>
#include <queue>
#include <glog/logging.h>
#include <util.h>
#include <atom/atom.h>
#include <core/core.h>
#include <index/real_index.h>

namespace {

const std::string k_default_index = "index";
const size_t k_stop_attempts = 120;
const size_t k_stop_interval_check_ms = 500;


std::string key(const Event& e) {
  return e.host() + "-" + e.service();
}

}

real_index::real_index(pub_sub & pubsub, push_event_fn_t push_event,
                       const int64_t expire_interval,
                       scheduler_interface &  sched,
                       spwan_thread_fn_t spwan_thread_fn)
:
  pubsub_(pubsub),
  push_event_fn_(push_event),
  expiring_(false),
  spwan_thread_fn_(spwan_thread_fn),
  sched_(sched)
{

  pubsub_.add_publisher(k_default_index,
                        std::bind(&real_index::all_events, this));

  sched_.add_periodic_task(std::bind(&real_index::timer_cb, this),
                            expire_interval);
}

std::vector<Event> real_index::all_events() {

  std::vector<Event> events;

  for (const auto & kv: index_map_) {
    events.push_back(kv.second);
  }

  return events;

}

void real_index::add_event(const Event& e) {

  VLOG(3) << "add_event()";

  const std::string ev_key(key(e));

  index_map_.erase(ev_key);
  index_map_.insert({ev_key, e});

  pubsub_.publish(k_default_index, e);
}

void real_index::timer_cb() {


  if (expiring_.exchange(true)) {

    VLOG(1) << "previous expire_events thread hasn't finished yet";
    return;

  }

  spwan_thread_fn_(std::bind(&real_index::expire_events, this));

}

void real_index::expire_events() {

  atom_attach_thread();

  VLOG(3) << "expire_fn()++";


  std::vector<std::string> keys_to_remove;
  std::vector<Event> expired_events;

  int64_t now = static_cast<int64_t>(sched_.unix_time());

  for (const auto & pair : index_map_) {

    const Event & event(pair.second);

    auto expire = event.time() + static_cast<int64_t>(event.ttl());
    if (expire < now) {
      keys_to_remove.push_back(pair.first);
      expired_events.push_back(event);
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

  VLOG(3) << "expire_fn()--";

  expiring_.store(false);

  atom_detach_thread();
}

real_index::~real_index() {

  VLOG(3) << "~real_index()++";

  if (expiring_.exchange(true)) {

    VLOG(3) << "expire_events thread is running";

    index_map_.clear();

    for (size_t attempts = k_stop_attempts; attempts > 0; attempts--) {

      if (!expiring_.exchange(true)) {
        break;
      }

      VLOG(3) << "Waiting for expiring event to finish";

      std::this_thread::sleep_for(
          std::chrono::milliseconds(k_stop_interval_check_ms));
    }

  }

  VLOG(3) << "~real_index()--";
}
