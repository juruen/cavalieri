#ifndef INDEX_H
#define INDEX_H

#include <unordered_map>
#include <set>
#include <proto.pb.h>
#include <pubsub.h>
#include <util.h>



typedef std::function<bool(const Event*, const Event*)> compare_t;
typedef std::set<Event*, compare_t> expire_t;
typedef expire_t::iterator expire_it_t;
typedef std::unordered_map<std::string, std::pair<Event, expire_it_t>> index_t;

class Index {
private:
  expire_t expire;
  PubSub& pubsub;
  index_t index;
  CallbackTimer* timer;

public:
  Index(PubSub& pubsub, const int64_t expire_interval);
  ~Index();
  void add_event(const Event& e);

private:
  void expire_events();
  const std::string key(const Event& e) const;
};

#endif
