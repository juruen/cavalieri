#ifndef CAVALIERI_STREAM_LIB_H
#define CAVALIERI_STREAM_LIB_H

#include <vector>
#include <memory>
#include <atomic>
#include <ctime>
#include <streams/stream_infra.h>

struct stream_lib {

  std::string file;
  std::time_t last_write_time;
  void * handle{nullptr};
  std::shared_ptr<streams_t> stream;
  std::atomic<unsigned int> ref_counter{0};

  bool used() const;
  void set_used(bool);
  bool inc();
  void dec();

};

#endif
