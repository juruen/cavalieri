#ifndef INSTRUMENTATION_RESERVOIR_H
#define INSTRUMENTATION_RESERVOIR_H

#include <atomic>
#include <atom/atom.h>

class reservoir {
public:
  typedef std::vector<double>  samples_t;

  reservoir();
  reservoir(const size_t size);
  void add_sample(const double sample);
  samples_t snapshot();

private:

  typedef struct {
    uint64_t n;
    samples_t samples;
  } reservoir_t;

  const size_t reservoir_size_;
  std::atomic<uint64_t> n_;
  atom<reservoir_t> reservoir_;

};

#endif
