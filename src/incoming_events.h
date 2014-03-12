#ifndef INCOMINIG_EVENTS
#define INCOMINIG_EVENTS

#include <streams/stream_functions.h>

class incoming_events {
public:
  incoming_events(streams & streams);
  void add_undecoded_msg(const std::vector<unsigned char> msgs);

private:
  streams streams_;
};

#endif
