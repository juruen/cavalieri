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
const size_t k_indexes = 8;

std::string key(const Event& e) {
  return e.host() + "-" + e.service();
}

const std::hash<std::string> hash_fn;

}

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
  indexes_(k_indexes),
  mutexes_(k_indexes)
{

  pubsub_.add_publisher(k_default_index,
                        std::bind(&real_index::all_events, this));

  sched_.add_periodic_task(std::bind(&real_index::timer_cb, this),
                            expire_interval);

}

std::vector<std::shared_ptr<Event>> real_index::all_events() {

  std::vector<std::shared_ptr<Event>> events;

  for (size_t i = 0; i < indexes_.size(); i++) {

    mutexes_[i].lock();
    auto index(indexes_[i]);
    mutexes_[i].unlock();

    for (const auto & kv : index) {
      events.push_back(kv.second);
    }

  }

  return events;
}

void real_index::add_event(const Event& e) {

  VLOG(3) << "add_event()";

  const std::string ev_key(key(e));

  const size_t index(hash_fn(ev_key) % indexes_.size());

  auto shared_event = std::make_shared<Event>(e);

  {
    std::lock_guard<std::mutex> lock(mutexes_[index]);
    indexes_[index][ev_key] = shared_event;
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
  return;

  atom_attach_thread();

  VLOG(3) << "expire_fn()++";

  /*
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
    event->set_state("expired");
    pubsub_.publish(k_default_index, *event);
    push_event_fn_(*event);
  }

  */
  VLOG(3) << "expire_fn()--";

  expiring_.store(false);

  atom_detach_thread();
}

real_index::~real_index() {

  VLOG(3) << "~real_index()++";

  if (expiring_.exchange(true)) {

    VLOG(3) << "expire_events thread is running";

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
