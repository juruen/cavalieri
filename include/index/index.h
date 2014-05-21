#ifndef INDEX_INDEX_H
#define INDEX_INDEX_H

#include <proto.pb.h>
#include <functional>
#include <memory>
#include <scheduler/scheduler.h>
#include <pub_sub/pub_sub.h>

typedef std::function<void(const Event &)> push_event_fn_t;
typedef std::function<void(std::function<void()>)> spwan_thread_fn_t;

class index_interface {
public:
  virtual void add_event(const Event & e) = 0;
  virtual ~index_interface() {};
};

#endif
