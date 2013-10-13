#include <index.h>
#include <util.h>
#include <glog/logging.h>

compare_t time_compare() {
  return [](const Event* lhs, const Event* rhs){
    return (lhs->time() <= rhs->time());
  };
}

Index::Index(PubSub& pubsub, const int64_t expire_interval) :
  expire(time_compare()),
  pubsub(pubsub)
{
  pubsub.add_publisher(
      "index",
      [&]() -> std::list<std::string> {
              std::list<std::string> evs;
              for (auto &kv: index) {
                evs.push_back(event_to_json(kv.second.first));
              }
              return evs;
            }
  );

  timer = new CallbackTimer(
      expire_interval,
      [&]() {
        VLOG(3) << "callback timer index";
        this->expire_events();
      });
}

Index::~Index() {

}

const std::string Index::key(const Event& e) const {
  return e.host() + "-" + e.service();
}

void Index::add_event(const Event& e) {
  VLOG(3) << "add_event()";

  auto idx_pair = index.insert({key(e), {e, expire.end()}});
  if (!idx_pair.second) {
    VLOG(3) << "updating event in index";
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

  VLOG(3) << "add_event() index size: " << index.size();
  VLOG(3) << "add_event() expire size: " << expire.size();
  pubsub.publish("index", event_to_json(e));

}

void Index::expire_events() {
  VLOG(3) << "expire_events()";
  int64_t now = static_cast<int64_t>(time(0));
  for (auto it = expire.begin(); it != expire.end(); it++) {
    const Event& event = **it;
    if ((event.time() + static_cast<int64_t>(event.ttl())) > now) {
      continue;
    }

    VLOG(3) << "event has expired: " << event_to_json(event);
    index.erase(key(event));
    expire.erase(it);

  }
}
