#ifndef INDEX_INDEX_H
#define INDEX_INDEX_H

#include <proto.pb.h>
#include <functional>

typedef std::function<void(const Event &)> push_event_fn_t;

class index_interface {
public:
  virtual void add_event(const Event & e) = 0;
};

class index {
public:
  index(index_interface & impl);
  void add_event(const Event& e);

private:
  index_interface & impl_;
};

#endif
