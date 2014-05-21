#include <glog/logging.h>
#include <streams/stream_infra.h>
#include <algorithm>

namespace {

forward_fn_t null_fn = [](const Event &)->void{};

streams_t child_join(streams_t left, streams_t right) {
  auto node = std::make_shared<stream_t>();

  streams_t list;

  for (const auto & n: left) {
    list.push_back(n);
  }

  for (const auto & n: right) {
    list.push_back(n);
  }

  auto first_fn = left.front()->input_fn;

  node->input_fn = [=](const Event & e) {
    left.front()->input_fn(e);
    right.front()->input_fn(e);
  };

  list.push_front(node);

  return list;
}

streams_t join(stream_node_t left, stream_node_t right) {
  left->output_fn = right->input_fn;

  streams_t list;
  list.push_back(left);
  list.push_back(right);

  return list;
}

streams_t join(streams_t left, streams_t right) {

  left.back()->output_fn = right.front()->input_fn;
  left.insert(left.end(), begin(right), end(right));

  return left;
}

streams_t join(streams_t left, stream_node_t right) {

  left.back()->output_fn = right->input_fn;
  left.push_back(right);

  return left;
}

}

streams_t join(stream_node_t left, streams_t right) {

  left->output_fn = right.front()->input_fn;
  right.push_front(left);

  return right;
}

streams_t create_stream(node_fn_t fn) {

  stream_node_t node = std::make_shared<stream_t>();

  std::weak_ptr<stream_t> weak_node = node;

  node->input_fn = [=](const Event & e) {
    fn(weak_node.lock()->output_fn, e);
  };

  return {node};
}

stream_t::stream_t() : output_fn(null_fn), input_fn(null_fn) {}

streams_t operator, (streams_t left, streams_t right) {
  return child_join(left, right);
}

streams_t operator>>(stream_node_t left, stream_node_t right) {
  return join(left, right);
}

streams_t operator>>(stream_node_t left, streams_t right) {
  return join(left, right);
}

streams_t operator>>(streams_t left, stream_node_t right) {
  return join(left, right);
}

streams_t operator>>(streams_t left, streams_t right) {
  return join(left, right);
}

void push_event(stream_node_t node, const Event & e) {
  node->input_fn(e);
}

void push_event(streams_t l, const Event & e) {
  l.front()->input_fn(e);
}
