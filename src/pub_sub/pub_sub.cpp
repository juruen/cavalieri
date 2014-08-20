#include <glog/logging.h>
#include <pub_sub/pub_sub.h>

void pub_sub::add_publisher(const std::string & topic)
{
  VLOG(3) << "add_publisher() topic: " << topic;

  publishers_.insert({topic, {}});
}

void pub_sub::publish(const std::string & topic, const Event & event) {

  VLOG(3) << "publish() topic: " << topic;

  auto it = publishers_.find(topic);

  CHECK(it != end(publishers_)) << "topic not found";

  it->second(event);

}

void pub_sub::subscribe(const std::string & topic,
                        const notify_event_fn_t notify_fn)
{
  VLOG(3) << "subscribe() topic: " << topic;

  auto it = publishers_.find(topic);

  CHECK(it != end(publishers_)) << "topic not found";

  it->second = notify_fn;

}
