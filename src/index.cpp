#include <thread>
#include <queue>
#include <glog/logging.h>
#include <util.h>
#include <index.h>
#include <scheduler.h>

index::index(
    pub_sub& pubsub,
    push_event_f_t push_event,
    const int64_t expire_interval)
  :
  pubsub(pubsub),
  push_event(push_event)
{
  pubsub.add_publisher(
      "index",
      [&]() -> std::vector<Event> {
              std::vector<Event> evs;
              for (auto const &kv: index_map) {
                evs.push_back(kv.second);
              }
              return evs;
            }
  );


  g_scheduler.add_periodic_task(
      [&]() {
        VLOG(3) << "callback timer index_map";
        this->expire_events();
      },
      expire_interval
  );
}

index::~index() {
}

const std::string index::key(const Event& e) const {
  return e.host() + "-" + e.service();
}

void index::add_event(const Event& e) {
  VLOG(3) << "add_event()";
  const std::string ev_key(key(e));
  index_map.erase(ev_key);
  index_map.insert({ev_key, e});
  pubsub.publish("index_map", e);
}

void index::expire_events() {
  auto expire_fn = [=]() {
    VLOG(3) << "expire_fn()";
    std::queue<std::string> to_erase;
    int64_t now = static_cast<int64_t>(time(0));
    for (auto const & pair : this->index_map) {
      const Event & event(pair.second);
      if (event.time() + static_cast<int64_t>(event.ttl()) < now) {
        to_erase.push(pair.first);
      }
    }
    while (!to_erase.empty()) {
      this->index_map.erase(to_erase.front());
      to_erase.pop();
    }
    VLOG(3) << "expire_fn() --";
  };

  std::thread thread(expire_fn);
  thread.detach();
}
