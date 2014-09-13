#ifndef CAVALIERI_INDEX_INDEX_H
#define CAVALIERI_INDEX_INDEX_H

#include <common/event.h>
#include <functional>
#include <memory>
#include <scheduler/scheduler.h>
#include <pub_sub/pub_sub.h>

using push_event_fn_t = std::function<void(const Event &)>;
using match_fn_t = std::function<bool(const Event &)>;
using spwan_thread_fn_t = std::function<void(std::function<void()>)>;

class index_interface {
public:
  virtual void add_event(const Event & e) = 0;
  virtual ~index_interface() {};
};

#endif
