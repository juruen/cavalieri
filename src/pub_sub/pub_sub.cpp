#include <glog/logging.h>
#include <pub_sub/pub_sub.h>

namespace {

std::vector<notify_queue_t> & notify_queues(publishers_t::iterator it) {
  return it->second.second;
}

all_events_fn_t & all_events_fn(publishers_t::iterator it) {
  return it->second.first;
}

}

void pub_sub::add_publisher(
    const std::string & topic,
    const all_events_fn_t & all_events_fn)
{
  VLOG(3) << "add_publisher() topic: " << topic;

  publishers_.insert({topic, {all_events_fn, {}}});
}

void pub_sub::publish(const std::string & topic, const Event & event) {

  VLOG(3) << "publish() topic: " << topic;

  auto it = publishers_.find(topic);

  CHECK(it != end(publishers_)) << "topic not found";

  for (auto & notify_queue : notify_queues(it)) {
    notify_queue->try_push(event);
  }

}

all_events_fn_t  pub_sub::subscribe(
    const std::string & topic,
    notify_queue_t notify_queue
)
{
  VLOG(3) << "subscribe() topic: " << topic;

  auto it = publishers_.find(topic);

  CHECK(it != end(publishers_)) << "topic not found";

  notify_queues(it).push_back(notify_queue);

  return all_events_fn(it);
}
