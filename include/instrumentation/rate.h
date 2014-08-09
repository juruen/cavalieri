#ifndef CAVALIERI_INSTRUMENTATION_RATE_H
#define CAVALIERI_INSTRUMENTATION_RATE_H

#include <atomic>
#include <chrono>

class rate {
public:

  rate();
  void add(unsigned int ticks);
  double snapshot();
  void reset();

private:
  std::atomic<unsigned int> counter_;
  std::chrono::high_resolution_clock::time_point start_;

};

#endif
