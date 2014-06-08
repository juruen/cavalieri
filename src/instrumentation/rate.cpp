#include <instrumentation/rate.h>

rate::rate() : counter_(0), start_(std::chrono::high_resolution_clock::now()) {}



void rate::add(unsigned int ticks) {
  counter_ += ticks;
}

double rate::snapshot() {

  auto counter = counter_.exchange(0);
  auto now = std::chrono::high_resolution_clock::now();

  auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(now -start_);
  auto rate = static_cast<double>(counter) / static_cast<double>(dt.count());

  start_ = now;

  return rate * 1000;

}

void rate::reset() {

  counter_.exchange(0);
  start_ = std::chrono::high_resolution_clock::now();


}
