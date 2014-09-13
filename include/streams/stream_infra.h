#ifndef CAVALIERI_STREAMS_STREAM_INFRA_H
#define CAVALIERI_STREAMS_STREAM_INFRA_H

#include <functional>
#include <list>
#include <boost/optional.hpp>
#include <common/event.h>

typedef std::vector<Event> next_events_t;

typedef std::function<void(next_events_t)> forward_fn_t;
typedef std::function<forward_fn_t()> fwd_new_stream_fn_t;

typedef std::function<next_events_t(const Event &)> on_event_fn_t;
typedef std::function<on_event_fn_t(fwd_new_stream_fn_t)> on_init_fn_t;
typedef std::function<on_event_fn_t()> on_init_simple_fn_t;


typedef struct {
  on_init_fn_t on_init;
  on_event_fn_t on_event;
} stream_node_t;

typedef std::vector<stream_node_t> streams_t;

streams_t create_stream(const on_event_fn_t);
streams_t create_stream(const on_init_fn_t);
streams_t create_stream(const on_init_simple_fn_t);


void init_streams(streams_t & streams);

next_events_t push_event(const streams_t &, const Event &);

streams_t operator>>(const streams_t & left , const streams_t & right);

#endif
