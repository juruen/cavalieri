#include <pubsub.h>
#include <glog/logging.h>
#include <util.h>

void pub_sub::add_publisher(
    const std::string& topic,
    const allevents_f_t& allevents_f)
{
  VLOG(3) << "add_publisher() topic: " << topic;
  publishers.insert({"index", allevents_f});
}

void pub_sub::publish(const std::string& topic, const Event& event) {
  VLOG(3) << "publish() topic: " << topic;
  for (auto  notify_queue : notify_queues_) {
    notify_queue->push(event);
  }
}

allevents_f_t  pub_sub::subscribe(
    const std::string& topic,
    notify_queue_t  notify_queue
)
{
  VLOG(3) << "subscribe() topic: " << topic;
  auto  pair = publishers.find("index");
  notify_queues_.push_back(notify_queue);

  return (pair->second);
}
