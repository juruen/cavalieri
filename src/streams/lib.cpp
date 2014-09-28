#include <streams/lib.h>

thread_local std::string * thread_ns = nullptr;

void free_thread_ns() {
  if (thread_ns != nullptr) {
    delete thread_ns;
    thread_ns = nullptr;
  }
}

void set_thread_ns(const std::string ns) {
  free_thread_ns();
  thread_ns = new std::string(ns);
}

void set_thread_global_ns() {
  free_thread_ns();
}

std::string get_thread_ns() {
  if (thread_ns) {
    return *thread_ns;
  } else {
    return k_global_ns;
  }
}

bool stream_lib::used() const {
  return ref_counter.load() & 1;
}

bool stream_lib::inc() {
  unsigned int current = ref_counter.load();
  unsigned int new_val;

  do {

    if (!(current & 1)) {
      // Stream is not being used. Abort inc
      return false;
    }

   new_val = (((current >> 1) + 1) << 1) | 1;

  } while(!ref_counter.compare_exchange_strong(current, new_val));

  return true;
}

void stream_lib::dec() {
  unsigned int current = ref_counter.load();
  unsigned int new_val;

  do {

   new_val = (((current >> 1) - 1) << 1) | (current & 1);

  } while(!ref_counter.compare_exchange_strong(current, new_val));
}

void stream_lib::set_used(const bool used) {
  if (used) {
    ref_counter |= 1;
  } else {
    ref_counter &= ~1;
  }
}
