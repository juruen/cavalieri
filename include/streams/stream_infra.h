#ifndef STREAMS_STREAM_INFRA_H
#define STREAMS_STREAM_INFRA_H

#include <functional>
#include <list>
#include <boost/optional.hpp>
#include <proto.pb.h>

typedef const std::function<void(const Event&)> forward_fn_t;

typedef std::function<void(const forward_fn_t, const Event &)> node_fn_t;

typedef std::list<node_fn_t> streams_t;

streams_t create_stream(const node_fn_t fn);

void push_event(const streams_t, const Event &);

streams_t operator>>(const streams_t left, const streams_t right);

#endif
