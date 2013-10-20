#include <pubsub.h>
#include <glog/logging.h>
#include <util.h>

void PubSub::add_publisher(const std::string& topic, const allevents_f_t& allevents_f) {
  VLOG(3) << "add_publisher() topic: " << topic;
  publishers.insert({topic, allevents_f});
}

void PubSub::publish(const std::string& topic, const Event& event) {
  std::string eventstr = event_to_json(event);
  VLOG(3) << "publish() topic: " << topic << " event: " << eventstr;
  auto it = subscribers.find(topic);
  if (it == subscribers.end()) {
    VLOG(3) << "no subscribers";
    return;
  }
  for (auto& f: it->second) {
    VLOG(3) << "notify";
    f({event});
  }
}

void PubSub::subscribe(
    const std::string& topic,
    const notify_f_t notify_f,
    const uintptr_t id)
{
  VLOG(3) << "subscribe() topic: " << topic;
  auto sub_it = subscribers.find(topic);
  if (sub_it == subscribers.end()) {
    VLOG(3) << "subscribe() first subscriber to topic, creating list";
    sub_it = subscribers.insert(sub_it, {topic, {}});
  }
  VLOG(3) << "subscribe() adding subscriber " << id << " to existing list";
  sub_it->second.push_back(notify_f);

  subscribers_iterators.insert({id, --(sub_it->second.end())});

  auto pub_it = publishers.find(topic);
  if (pub_it == publishers.end()) {
    VLOG(3) << "subscribe() not publisher for this topic yet";
    return;
  }
  auto f = pub_it->second;
  VLOG(3) << "subscribe() fetching all events";
  const evs_list_t evs = f();
  VLOG(3) << "subscribe() sending events";
  notify_f(evs);
}

void PubSub::unsubscribe(const std::string& topic, const uintptr_t id) {
  VLOG(3) << "unsubscribe() id: " << id << " from topic: " << topic;

  auto sub_it_it = subscribers_iterators.find(id);
  if (sub_it_it == subscribers_iterators.end()) {
    VLOG(3) << "id doesn't exist";
    return;
  }

  auto sub_it = subscribers.find(topic);
  if (sub_it == subscribers.end())  {
    VLOG(3) << "topic doesn't exist";
    return;
  }

  VLOG(3) << "using iterator to unsubscribe";
  sub_it->second.erase(sub_it_it->second);

  if (sub_it->second.size() == 0) {
    VLOG(3) << "subscriber list empty, removing topic";
    subscribers.erase(sub_it);
  }
}
