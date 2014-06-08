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
    reservoir_size_(k_default_size)
{
}

reservoir::reservoir(const size_t size)
  :
    reservoir_size_(size)
{
}

void reservoir::add_sample(const double sample)
{

  VLOG(3) << "add_sample";

  reservoir_.update(
      [&](const reservoir_t & current) {

        if (current.n < reservoir_size_) {

          VLOG(3) << "not full";

          auto copy(current);
          copy.samples.push_back(sample);
          copy.n++;

          return copy;

        } else {

          VLOG(3) << "full";

          std::uniform_int_distribution<> distr(0, current.n);

          auto index = distr(eng);
          if (static_cast<size_t>(index)  < reservoir_size_) {

            auto copy(current);
            copy.n++;
            copy.samples[index] = sample;

            return copy;

          }

        }

        return current;

      });
}


std::vector<double> reservoir::snapshot() {

  samples_t copy;

  reservoir_.update(
      [&](const reservoir_t &) {

          return reservoir_t({0, samples_t(reservoir_size_)});

      },
      [&](const reservoir_t & old, const reservoir_t &) {

        copy =  old.samples;

      }
          
      );

  return copy;

}
