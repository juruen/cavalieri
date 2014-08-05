#ifndef INSTRUMENTATION_RESERVOIR_H
#define INSTRUMENTATION_RESERVOIR_H

#include <atomic>
#include <atom/atom.h>
#include <mutex>

class reservoir {
public:
  typedef std::vector<double>  samples_t;

  reservoir();
  reservoir(const size_t size);
  void add_sample(const double sample);
  samples_t snapshot();

private:

  const size_t reservoir_size_;
  samples_t samples_;
  std::atomic<uint64_t> n_;
  std::mutex mutex_;

};

#endif
