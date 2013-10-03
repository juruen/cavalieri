#ifndef STREAMS_H
#define STREAMS_H

#include <list>
#include <functional>
#include <proto.pb.h>

typedef std::function<void(const Event&)> stream_t;
typedef std::list<stream_t> childrent_t;

struct Streams {
  std::list<stream_t> streams;
  void add_stream(stream_t stream);
  void process_message(const Msg& message);
  static stream_t prn();
};

#endif
