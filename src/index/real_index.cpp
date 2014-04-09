#include <thread>
#include <queue>
#include <glog/logging.h>
#include <util.h>
#include <atom/atom.h>
#include <index/real_index.h>
#include <scheduler/scheduler.h>

namespace {

const std::string k_default_index = "index";

std::string key(const Event& e) {
  return e.host() + "-" + e.service();
}

}

real_index::real_index(pub_sub & pubsub, push_event_fn_t push_event,
                       const int64_t expire_interval,
                       spwan_thread_fn_t spwan_thread_fn)
:
  pubsub_(pubsub),
  push_event_fn_(push_event),
  expiring_(false),
  spwan_thread_fn_(spwan_thread_fn)
{

  pubsub_.add_publisher(k_default_index,
                        std::bind(&real_index::all_events, this));

  g_scheduler.add_periodic_task(std::bind(&real_index::timer_cb, this),
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

  if (expiring_ == true) {

    VLOG(1) << "previous expire_events thread hasn't finished yet";
    return;

  }

  expiring_ = true;

  spwan_thread_fn_(std::bind(&real_index::expire_events, this));
}

void real_index::expire_events() {

  atom_attach_thread();

  VLOG(3) << "expire_fn()++";


  std::vector<std::string> keys_to_remove;
  std::vector<Event> expired_events;

  int64_t now = static_cast<int64_t>(g_scheduler.unix_time());

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
    push_event_fn_(event);
  }

  VLOG(3) << "expire_fn()--";

  expiring_ = false;

  atom_detach_thread();
}

real_index::~real_index() {
  // TODO: Make sure the periodic task is stopped
}


class index create_index(pub_sub & pubsub, push_event_fn_t push_event,
                         const int64_t expire_interval,
                         spwan_thread_fn_t spwan_thread_fn)
{
  auto real_idx = std::make_shared<real_index>(pubsub, push_event,
                                               expire_interval,
                                               spwan_thread_fn);

  auto idx_iface = std::dynamic_pointer_cast<index_interface>(real_idx);

  class index idx(idx_iface);

  return idx;
}


