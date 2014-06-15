#include <glog/logging.h>
#include <instrumentation/reservoir.h>
#include <random>

namespace {

const size_t k_default_size = 1000;

std::random_device rd;
std::mt19937_64 eng(rd());

}

reservoir::reservoir()
  :
    reservoir_size_(k_default_size),
    samples_(k_default_size, 0),
    n_(0)
{
}

reservoir::reservoir(const size_t size)
  :
    reservoir_size_(size),
    samples_(size, 0),
    n_(0)
{
}

void reservoir::add_sample(const double sample)
{

  VLOG(1) << "add_sample";

  if (uint64_t i= n_.fetch_add(1) < reservoir_size_) {

    std::lock_guard<std::mutex> lock(mutex_);
    samples_[i] = sample;

  } else {

    std::uniform_int_distribution<> distr(0, n_);

    auto index = distr(eng);
    if (static_cast<size_t>(index)  < reservoir_size_) {
      std::lock_guard<std::mutex> lock(mutex_);
      samples_[index] = sample;
    }

    n_++;

  }

}

std::vector<double> reservoir::snapshot() {

  std::lock_guard<std::mutex> lock(mutex_);
  return samples_;

}
