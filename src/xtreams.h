#ifndef XTREAMS_H
#define XTREAMS_H

#include <functional>
#include <vector>
#include <list>
#include <iostream>
#include <memory>
#include <proto.pb.h>

typedef std::function<void(const Event&)> forward_fn_t;
typedef std::function<void(const std::vector<Event>)> mforward_fn_t;

struct xtream_t {
  xtream_t();
  forward_fn_t output_fn;
  forward_fn_t input_fn;
};

typedef std::shared_ptr<xtream_t> xtream_node_t;

struct min_xtream_t {
  min_xtream_t();
  forward_fn_t output_fn;
  mforward_fn_t input_fn;
};

typedef std::shared_ptr<min_xtream_t> min_xtream_node_t;

struct mout_xtream_t {
  mout_xtream_t();
  mforward_fn_t output_fn;
  forward_fn_t input_fn;
};

typedef std::shared_ptr<mout_xtream_t> mout_xtream_node_t;

typedef std::list<xtream_node_t> xtreams_t;

typedef std::function<void(forward_fn_t, const Event &)> node_fn_t;

typedef std::function<void(forward_fn_t, std::vector<Event>)> min_node_fn_t;

typedef std::function<void(mforward_fn_t, const Event &)> mout_node_fn_t;

xtream_node_t create_xtream_node(node_fn_t fn);

min_xtream_node_t create_min_xtream_node(min_node_fn_t fn);

mout_xtream_node_t create_mout_xtream_node(mout_node_fn_t fn);

void push_event(xtream_node_t, const Event &);

void push_event(xtreams_t, const Event &);

xtream_node_t operator+ (xtream_node_t left, xtream_node_t right);

xtreams_t operator>>(mout_xtream_node_t left, min_xtream_node_t right);

xtreams_t operator>(xtream_node_t left, xtream_node_t right);

xtreams_t operator>(xtream_node_t left, xtreams_t right);

xtreams_t operator>(xtreams_t left, xtream_node_t right);

xtreams_t operator>(xtreams_t left, xtreams_t right);

#endif
