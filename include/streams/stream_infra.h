#ifndef XTREAMS_H
#define XTREAMS_H

#include <functional>
#include <vector>
#include <list>
#include <iostream>
#include <memory>
#include <proto.pb.h>

typedef std::function<void(const Event&)> forward_fn_t;

struct stream_t {
  stream_t();
  forward_fn_t output_fn;
  forward_fn_t input_fn;
};

typedef std::shared_ptr<stream_t> stream_node_t;

typedef std::list<stream_node_t> streams_t;

typedef std::function<void(forward_fn_t, const Event &)> node_fn_t;

streams_t create_stream(node_fn_t fn);

void push_event(stream_node_t, const Event &);

void push_event(streams_t, const Event &);

streams_t operator, (streams_t left, streams_t right);

streams_t operator>>(stream_node_t left, stream_node_t right);

streams_t operator>>(stream_node_t left, streams_t right);

streams_t operator>>(streams_t left, stream_node_t right);

streams_t operator>>(streams_t left, streams_t right);

#endif
