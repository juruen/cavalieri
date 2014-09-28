#ifndef CAVALIERI_STREAM_LIB_H
#define CAVALIERI_STREAM_LIB_H

#include <vector>
#include <memory>
#include <atomic>
#include <ctime>
#include <streams/stream_infra.h>

const std::string k_global_ns("global");

/* This thread_local variable is used to track the lib namespace that is using
 * the thread. It is useful to unload the library completely and remove
 * all code that belongs to the library
 */
extern thread_local std::string * thread_ns;

void free_thread_ns();

void set_thread_ns(const std::string ns);

void set_thread_global_ns();

std::string get_thread_ns();

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
