#include <glog/logging.h>
#include <streams/stream_infra.h>
#include <algorithm>
#include <iostream>
#include <util.h>

namespace {

streams_t join(const streams_t left, const streams_t right) {

  streams_t stream;

  stream.insert(stream.end(), left.begin(), left.end());
  stream.insert(stream.end(), right.begin(), right.end());

  return stream;
}

}

streams_t create_stream(const node_fn_t fn) {

  return {fn};

}


streams_t operator>>(streams_t left, streams_t right) {
  return join(left, right);
}

void push_event(const streams_t stream, const Event & event) {
  std::vector<Event> forwarded_events({event});
  bool forward;

  forward_fn_t forward_fn = [&](const Event & e)
  {
    forward = true;
    forwarded_events.push_back(e);
  };

  for (const auto & node: stream) {

    forward = false;

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
