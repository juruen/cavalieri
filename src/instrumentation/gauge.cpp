#include <instrumentation/gauge.h>

gauge::gauge() : gauge_(0) {};

void gauge::update(unsigned int ticks) {
  gauge_.store(ticks);
}

void gauge::incr(unsigned int ticks) {
  gauge_ += ticks;
}

void gauge::decr(unsigned int ticks) {
  gauge_ -= ticks;
}

unsigned int gauge::snapshot() {
  return gauge_.load();
}
