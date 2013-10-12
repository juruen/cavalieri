#include <index.h>
#include <util.h>
#include <glog/logging.h>

ExpireCompare::ExpireCompare(const bool& revparam) : reverse(revparam) {}

bool ExpireCompare::operator() (const expire_t& lhs, const expire_t& rhs) const {
    if (reverse) {
       return (lhs.first.time() > rhs.first.time());
    } else {
       return (lhs.first.time() < rhs.first.time());
    }
};

Index::Index(PubSub& pubsub) : expire(ExpireCompare(false)), pubsub(pubsub) {
  pubsub.add_publisher(
      "index",
      [&]() -> std::list<std::string> {
              std::list<std::string> evs;
              for (auto &kv: index) {
                auto &v = kv.second;
                evs.push_back(event_to_json(v));
              }
              return evs;
            }
  );
}

void Index::add_event(const Event& e) {
  VLOG(3) << "add_event()";
  index.insert({e.host() + "-" + e.service(), e});
  VLOG(3) << "add_event() index size: " << index.size();
  pubsub.publish("index", event_to_json(e));
}
