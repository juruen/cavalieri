#ifndef INDEX_INDEX_H
#define INDEX_INDEX_H

#include <proto.pb.h>
#include <functional>
#include <memory>
#include <pub_sub/pub_sub.h>

typedef std::function<void(const Event &)> push_event_fn_t;
typedef std::function<void(std::function<void()>)> spwan_thread_fn_t;

class index_interface {
public:
  virtual void add_event(const Event & e) = 0;
};

class index {
public:
  index(std::shared_ptr<index_interface> impl);
  void add_event(const Event& e);

private:
  std::shared_ptr<index_interface> impl_;
};


class index create_index(pub_sub & pubsub, push_event_fn_t push_event,
                         const int64_t expire_interval,
                         spwan_thread_fn_t spwan_thread_fn);

#endif
