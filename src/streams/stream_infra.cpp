#include <glog/logging.h>
#include <streams/stream_infra.h>
#include <algorithm>
#include <iostream>
#include <util/util.h>

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

next_events_t push_event(const streams_t & stream, const Event & event) {

  next_events_t next_events;

  for (const auto & node: stream) {

    if (next_events.empty()) { // First iteration

      next_events = node(event);

    } else {

      const auto aux_next_events(next_events);
      next_events.clear();

      for (const auto & e : aux_next_events) {

        const auto res = node(e);
        std::copy(begin(res), end(res), back_inserter(next_events));

      }

    }

    if (next_events.empty()) {
      return {};
    }

  }

  return next_events;
}
