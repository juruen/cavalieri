#ifndef REAL_INDEX_H
#define REAL_INDEX_H

#include <proto.pb.h>
#include <tbb/concurrent_hash_map.h>
#include <pubsub.h>
#include <util.h>
#include <index.h>

typedef tbb::concurrent_hash_map<std::string, Event> real_index_t;

class real_index : public index_interface {
public:
  real_index(
      pub_sub& pubsub,
      push_event_fn_t push_event,
      const int64_t expire_interval
    );
  ~real_index();
  void add_event(const Event& e);

private:
  void expire_events();
  const std::string key(const Event& e) const;

private:
  pub_sub& pubsub;
  real_index_t index_map;
  push_event_fn_t push_event;
};

#endif
