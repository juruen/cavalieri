#include <thread>
#include <queue>
#include <glog/logging.h>
#include <util/util.h>
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
const size_t k_indexes = 8;

std::string key(const Event& e) {
  return e.host() + "-" + e.service();
}

const std::hash<std::string> hash_fn;

}

real_index::real_index(pub_sub & pubsub, push_event_fn_t push_event,
                       const int64_t expire_interval,
                       scheduler_interface &  sched,
                       instrumentation::instrumentation & instr,
                       spwan_thread_fn_t spwan_thread_fn)
:
  pubsub_(pubsub),
  pre_gauge_(instr.add_gauge(k_preexpire_service, k_preexpire_desc)),
  post_gauge_(instr.add_gauge(k_postexpire_service, k_postexpire_desc)),
  push_event_fn_(push_event),
  expiring_(false),
  spwan_thread_fn_(spwan_thread_fn),
  sched_(sched),
  indexes_(k_indexes),
  mutexes_(k_indexes)
{

  pubsub_.add_publisher(k_default_index);

  sched_.add_periodic_task(std::bind(&real_index::timer_cb, this),
                            expire_interval);

}

void real_index::add_event(const Event& e) {

  VLOG(3) << "add_event()";

  const std::string ev_key(key(e));

  const size_t index(hash_fn(ev_key) % indexes_.size());

  {
    std::lock_guard<std::mutex> lock(mutexes_[index]);
    indexes_[index][ev_key] = e;
  }

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

  VLOG(3) << "expire_fn()++";

  std::vector<Event> expired_events;

  int64_t now = static_cast<int64_t>(sched_.unix_time());

  size_t index_size = 0;

  for (size_t i = 0; i < indexes_.size(); i++) {

    std::lock_guard<std::mutex> lock(mutexes_[i]);

    index_size += indexes_[i].size();

    auto it = indexes_[i].begin();
    while (it != indexes_[i].end()) {

      const auto & event(it->second);

      auto expire = event.time() + static_cast<int64_t>(event.ttl());
      if (expire < now) {
        expired_events.push_back(event);
        indexes_[i].erase(it++);
      } else {
        ++it;
      }

    }

  }

  pre_gauge_.update_fn(index_size);
  post_gauge_.update_fn(index_size - expired_events.size());

  VLOG(3) << "expire process took "
          << static_cast<int64_t>(sched_.unix_time()) - now << " seconds";

  for (auto & event : expired_events) {
    event.set_state("expired");
    pubsub_.publish(k_default_index, event);
    push_event_fn_(event);
  }

  VLOG(3) << "expire_fn()--";

  expiring_.store(false);

}

std::vector<Event> real_index::query_index(const match_fn_t match_fn,
                                           const size_t max_matches)
{

  VLOG(3) << "quering index";

  std::vector<Event> events;

  size_t matches = 0;

  for (size_t i = 0; i < indexes_.size(); i++) {

    std::lock_guard<std::mutex> lock(mutexes_[i]);

    for (const auto & event : indexes_[i]) {

      if (match_fn(event.second)) {

        events.push_back(event.second);

        matches++;

        if (matches == max_matches) {
          return std::move(events);
        }

      }

    }

  }

  VLOG(3) << "matches: " << matches;

  return std::move(events);
}

real_index::~real_index() {

  VLOG(3) << "~real_index()++";

  if (expiring_.exchange(true)) {

    VLOG(3) << "expire_events thread is running";

    for (size_t i = 0; i < indexes_.size(); i++) {

      std::lock_guard<std::mutex> lock(mutexes_[i]);
      indexes_[i].clear();

    }

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
