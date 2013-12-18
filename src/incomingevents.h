#ifndef INCOMINIG_EVENTS
#define INCOMINIG_EVENTS

#include <streams.h>

class incoming_events {
public:
  incoming_events(streams & streams_);
  void add_undecoded_msg(std::vector<unsigned char> msgs);

private:
  streams & streams_;
};

#endif
