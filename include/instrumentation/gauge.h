#ifndef CAVALIERI_INSTRUMENTATION_GAUGE_H
#define CAVALIERI_INSTRUMENTATION_GAUGE_H

#include <atomic>

class gauge {
public:

  gauge();

  void update(unsigned int ticks);
  void incr(unsigned int ticks);
  void decr(unsigned int ticks);

  unsigned int snapshot();

private:

  std::atomic<unsigned int> gauge_;

};

#endif
