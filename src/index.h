#ifndef INDEX_H
#define INDEX_H

#include <proto.pb.h>
#include <tbb/concurrent_hash_map.h>
#include <pubsub.h>
#include <util.h>



typedef tbb::concurrent_hash_map<std::string, Event> index_t;
typedef std::function<void(const Event&)> push_event_f_t;

class index {
private:
  pub_sub& pubsub;
  index_t index_map;
  push_event_f_t push_event;

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
