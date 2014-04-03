#ifndef PUB_SUB_H
#define PUB_SUB_H

#include <unordered_map>
#include <functional>
#include <string>
#include <proto.pb.h>
#include <vector>
#include <tbb/concurrent_queue.h>
#include <memory>

typedef std::function<std::vector<Event>()> all_events_fn_t;;
typedef std::shared_ptr<tbb::concurrent_bounded_queue<Event>> notify_queue_t;
typedef std::pair<all_events_fn_t, std::vector<notify_queue_t>> publisher_data_t;
typedef std::unordered_map<std::string, publisher_data_t> publishers_t;


class pub_sub {
public:

  void add_publisher(const std::string & topic,
                     const all_events_fn_t & all_events_fn);

  void publish(const std::string & topic, const Event & event);

  all_events_fn_t subscribe(const std::string & topic,
                            notify_queue_t notify_queue);

private:
  publishers_t publishers_;
};

#endif
