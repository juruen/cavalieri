#include <pubsub.h>
#include <glog/logging.h>

void PubSub::add_publisher(const std::string& topic, const allevents_f_t& allevents_f) {
  VLOG(3) << "add_publisher() topic: " << topic;
  publishers.insert({topic, allevents_f});
}

void PubSub::publish(const std::string& topic, const std::string eventstr) {
  VLOG(3) << "publish() topic: " << topic << " event: " << eventstr;
  auto it = subscribers.find(topic);
  if (it == subscribers.end()) {
    VLOG(3) << "no subscribers";
    return;
  }
  for (auto& f: it->second) {
    VLOG(3) << "notify";
    f({}, eventstr);
  }
}

void PubSub::subscribe(const std::string& topic, const notify_f_t notify_f) {
  VLOG(3) << "subscribe() topic: " << topic;
  auto it_sub = subscribers.find(topic);
  if (it_sub == subscribers.end()) {
    VLOG(3) << "subscribe() first subscriber";
    subscribers.insert({topic, {notify_f}});
  } else {
    VLOG(3) << "subscribe() adding subscriber to list";
    it_sub->second.push_back(notify_f);
  }

  auto it_pub = publishers.find(topic);
  if (it_pub == publishers.end()) {
    VLOG(3) << "subscribe() not publisher for this topic yet";
    return;
  }
  auto f = it_pub->second;
  VLOG(3) << "subscribe() fetching all events";
  const evstr_list_t evs = f();
  VLOG(3) << "subscribe() sending events";
  notify_f(evs, "");
}
