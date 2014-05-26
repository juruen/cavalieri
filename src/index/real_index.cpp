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

  const std::string ev_key(key(e));

  auto shared_event = std::make_shared<Event>(e);

  {
    std::lock_guard<std::mutex> lock(mutex_);
    index_map_.insert({ev_key, shared_event});
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

  atom_attach_thread();

  VLOG(3) << "expire_fn()++";

  VLOG(3) << "index size: " << index_map_.size();

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

  VLOG(3) << "expire process took "
          << static_cast<int64_t>(sched_.unix_time()) - now << " seconds";

  for (auto & event : expired_events) {
    event->set_state("expired");
    pubsub_.publish(k_default_index, *event);
    push_event_fn_(*event);
  }

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
