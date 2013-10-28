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
typedef std::function<void(const Event&)> push_event_f_t;

class index {
private:
  expire_t expire;
  pub_sub& pubsub;
  index_t index_map;
  push_event_f_t push_event;
  callback_timer* timer;

public:
  index(
      pub_sub& pubsub,
      push_event_f_t push_event,
      const int64_t expire_interval
    );
  ~index();
  void add_event(const Event& e);

private:
  void expire_events();
  const std::string key(const Event& e) const;
};

#endif
