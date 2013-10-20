#ifndef PUBSUB_H
#define PUBSUB_H

#include <unordered_map>
#include <functional>
#include <string>
#include <proto.pb.h>
#include <vector>

typedef std::vector<Event> evs_list_t;
typedef std::function<evs_list_t()> allevents_f_t;
typedef std::unordered_map<std::string, allevents_f_t> publishers_t;
typedef std::function<void(const evs_list_t&)> notify_f_t;
typedef std::vector<notify_f_t> notify_f_list_t;
typedef std::unordered_map<std::string, notify_f_list_t> subscribers_t;
typedef std::unordered_map<uintptr_t, notify_f_list_t::iterator> subscribers_iterators_t;

class PubSub {
private:
  publishers_t publishers;
  subscribers_t subscribers;
  subscribers_iterators_t subscribers_iterators;

public:
  void add_publisher(const std::string& topic, const allevents_f_t& allevents_f);
  void publish(const std::string& topic, const Event& eventt);
  void subscribe(const std::string& topic, const notify_f_t notify_f, const uintptr_t id);
  void unsubscribe(const std::string& topic, const uintptr_t id);
};

#endif
