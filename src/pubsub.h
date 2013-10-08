#ifndef PUBSUB_H
#define PUBSUB_H

#include <unordered_map>
#include <functional>
#include <string>
#include <proto.pb.h>
#include <list>

typedef std::list<std::string> evstr_list_t;
typedef std::function<evstr_list_t()> allevents_f_t;
typedef std::unordered_map<std::string, allevents_f_t> publishers_t;
typedef std::function<void(const evstr_list_t&, const std::string)> notify_f_t;
typedef std::unordered_map<std::string, std::list<notify_f_t>> subscribers_t;

class PubSub {
private:
  publishers_t publishers;
  subscribers_t subscribers;

public:
  void add_publisher(const std::string& topic, const allevents_f_t& allevents_f);
  void publish(const std::string& topic, const std::string eventstr);
  void subscribe(const std::string& topic, const notify_f_t notify_f);
};

#endif
