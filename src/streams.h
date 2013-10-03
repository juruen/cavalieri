#ifndef STREAMS_H
#define STREAMS_H

#include <list>
#include <functional>
#include <proto.pb.h>

typedef std::function<void(const Event&)> stream_t;
typedef std::list<stream_t> children_t;
typedef std::map<std::string, std::string> with_changes_t;


void call_rescue(const Event& e, const children_t& children);

stream_t prn();
stream_t with(const with_changes_t& changes, const children_t& children);

struct Streams {
  std::list<stream_t> streams;
  void add_stream(stream_t stream);
  void process_message(const Msg& message);
};

#endif
