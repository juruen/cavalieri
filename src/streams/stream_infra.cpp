#include <glog/logging.h>
#include <streams/stream_infra.h>
#include <algorithm>
#include <iostream>
#include <util.h>

namespace {

streams_t join(const streams_t & left, const streams_t & right) {

  streams_t stream;

  stream.insert(stream.end(), left.begin(), left.end());
  stream.insert(stream.end(), right.begin(), right.end());

  return std::move(stream);
}

}

streams_t create_stream(const node_fn_t & fn) {

  return {fn};

}


streams_t operator>>(const streams_t & left, const streams_t & right) {
  return std::move(join(left, right));
}

void push_event(const streams_t & stream, const Event & event) {

  std::vector<Event> forwarded_events;

  bool forward;
  bool first_event = true;

  forward_fn_t forward_fn = [&](const Event & e)
  {
    forward = true;
    forwarded_events.push_back(e);
  };

  for (const auto & node: stream) {

    forward = false;

    // FIXME This a dirty optimization in the critical path
    // to avoid copying event in the first stream
    if (first_event) {
      node(forward_fn, event);
      if (forward) {
        first_event = false;
        continue;
      } else {
        return;
      }
    }

    auto f_events(std::move(forwarded_events));
    for (const auto & f_event: f_events) {
      node(forward_fn, f_event);
    }

    if (!forward) {
      return;
    }

  }

  return;
}
