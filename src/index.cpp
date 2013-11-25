#include <util.h>
#include <glog/logging.h>
#include <index.h>

compare_t time_compare() {
  return [](const Event* lhs, const Event* rhs){
    return (lhs->time() <= rhs->time());
  };
}

index::index(
    pub_sub& pubsub,
    push_event_f_t push_event,
    const int64_t expire_interval)
  :
  expire(time_compare()),
  pubsub(pubsub),
  push_event(push_event)
{
  pubsub.add_publisher(
      "index_map",
      [&]() -> std::vector<Event> {
              std::vector<Event> evs;
              for (auto const &kv: index_map) {
                evs.push_back(kv.second.first);
              }
              return evs;
            }
  );

  timer = new callback_timer(
      expire_interval,
      [&]() {
        VLOG(3) << "callback timer index_map";
        this->expire_events();
      });
}

index::~index() {
  delete timer;
}

const std::string index::key(const Event& e) const {
  return e.host() + "-" + e.service();
}

void index::add_event(const Event& e) {
  VLOG(3) << "add_event()";

  auto idx_pair = index_map.insert({key(e), {e, expire.end()}});
  if (!idx_pair.second) {
    VLOG(3) << "updating event in index_map";
    idx_pair.first->second.first = e;
    VLOG(3) << "removing event from expire";
    expire.erase(idx_pair.first->second.second);
  }

  VLOG(3) << "adding event to expire with time: " << e.time()
          << " and pointer: " << &idx_pair.first->second.first;

  auto exp_pair = expire.insert(&idx_pair.first->second.first);
  if (!exp_pair.second) {
    LOG(ERROR) << "something wrong with expire set";
  }
  idx_pair.first->second.second = exp_pair.first;

  VLOG(3) << "add_event() idx size: " << index_map.size();
  VLOG(3) << "add_event() expire size: " << expire.size();

  pubsub.publish("index_map", e);

}

void index::expire_events() {
  VLOG(3) << "expire_events() expire size: " << expire.size();
  int64_t now = static_cast<int64_t>(time(0));
  std::vector<expire_t::iterator> to_remove;
  for (auto it = expire.begin(); it != expire.end(); it++) {
    const Event& event = **it;
    if ((event.time() + static_cast<int64_t>(event.ttl())) > now) {
      continue;
    }
    VLOG(3) << "event has expired: " << event_to_json(event);
    Event nevent = event;
    nevent.set_state("expired");
    VLOG(3) << "pushing expired event to streams";
    push_event(nevent);
    to_remove.push_back(it);
  }
  VLOG(3) << "removing events";
  for (auto it : to_remove) {
    std::string key_to_remove = key(**it);
    expire.erase(it);
    index_map.erase(key_to_remove);
  }
  VLOG(3) << "expire_events() --";
}
