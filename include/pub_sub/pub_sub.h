#ifndef CAVALIERI_PUB_SUB_H
#define CAVALIERI_PUB_SUB_H

#include <unordered_map>
#include <functional>
#include <string>
#include <common/event.h>
#include <vector>
#include <tbb/concurrent_queue.h>
#include <memory>

typedef std::function<void(const Event &)> notify_event_fn_t;
typedef std::unordered_map<std::string, notify_event_fn_t> publishers_t;


class pub_sub {
public:

  void add_publisher(const std::string & topic);

  void publish(const std::string & topic, const Event & event);

  void subscribe(const std::string & topic,
                 const notify_event_fn_t notify_event);

private:
  publishers_t publishers_;
};

#endif
