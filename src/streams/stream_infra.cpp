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

fwd_new_stream_fn_t forward(streams_t::iterator begin, streams_t::iterator end)
{

  streams_t s;
  std::copy(begin, end, back_inserter(s));

  return [=]() mutable -> forward_fn_t
  {

    init_streams(s);

    return [=](next_events_t events)
    {
      for (const auto & event : events) {
        push_event(s, event);
      }
    };

  };

}

}

streams_t create_stream(const on_event_fn_t fn) {

  return {{[=](fwd_new_stream_fn_t){ return fn; }, fn}};

}

streams_t create_stream(const on_init_simple_fn_t fn) {

  return {{[=](fwd_new_stream_fn_t){ return fn(); }, {}}};

}

streams_t create_stream(const on_init_fn_t fn) {

  return {{fn, {}}};

}



void init_streams(streams_t & streams) {

  for (size_t j = 0, i = streams.size() - 1; j < streams.size() ; i--, j++) {

    auto & node = streams[i];

    node.on_event = node.on_init(forward(streams.end() - j , streams.end()));

  }

}

streams_t operator>>(const streams_t & left, const streams_t & right) {
  return std::move(join(left, right));
}

next_events_t push_event(const streams_t & stream, const Event & event) {

  next_events_t next_events;

  for (const auto & node: stream) {

    if (next_events.empty()) { // First iteration

      next_events = node.on_event(event);

    } else {

      const auto aux_next_events(next_events);
      next_events.clear();

      for (const auto & e : aux_next_events) {

        const auto res = node.on_event(e);
        std::copy(begin(res), end(res), back_inserter(next_events));

      }

    }

    if (next_events.empty()) {
      return {};
    }

  }

  return next_events;
}
