#ifndef PUBSUB_H
#define PUBSUB_H

#include <unordered_map>
#include <functional>
#include <string>
#include <proto.pb.h>
#include <vector>
#include <tbb/concurrent_queue.h>
#include <memory>

typedef std::vector<Event> evs_list_t;
typedef std::function<evs_list_t()> allevents_f_t;
typedef std::unordered_map<std::string, allevents_f_t> publishers_t;
typedef std::shared_ptr<tbb::concurrent_queue<Event>> notify_queue_t;

class pub_sub {
public:
  void add_publisher(const std::string& topic, const allevents_f_t& allevents_f);
  void publish(const std::string& topic, const Event& eventt);
  allevents_f_t subscribe(
      const std::string& topic,
      notify_queue_t  notify_queue);

private:
  publishers_t publishers;
  std::vector<notify_queue_t> notify_queues_;
};

#endif
