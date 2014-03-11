#include <folds.h>
#include <functional>
#include <util.h>

namespace {

typedef std::function<double(double, double)> reduce_fn_t;
typedef std::vector<Event> events_t;
typedef std::function<Event(const events_t)> fold_fn_t;

double reduce(const reduce_fn_t f, const events_t & events) {

  double t(metric_to_double(events[0]));

  for (size_t i = 1; i < events.size(); i++) {

    t = f(t, metric_to_double(events[i]));

  }

  return t;
}

double sum_fn(const double & x, const double & y) {
  return x + y;
}

double product_fn(const double & x, const double & y) {
  return x * y;
}

double difference_fn(const double & x, const double & y) {
  return x - y;
}

fold_result_t fold(const reduce_fn_t f, events_t events) {

    //TODO: Filter nil metric events
    if (events.empty()) {
      return {};
    }

    return reduce(f, events);
}

}


fold_result_t sum(events_t events) {
  return fold(sum_fn, events);
}

fold_result_t product(events_t events) {
  return fold(product_fn, events);
}

fold_result_t difference(events_t events) {
  return fold(difference_fn, events);
}

fold_result_t mean(events_t events) {

    if (events.empty()) {
      return {};
    }


   return reduce(sum_fn, events) / events.size();
}

fold_result_t minimum(events_t events) {

    if (events.empty()) {
      return {};
    }

    double min = metric_to_double(events[0]);

    for (const auto & e: events) {

      auto tmp = metric_to_double(e);
      if (tmp < min) {
        min = tmp;
      }

    }

    return min;
}

fold_result_t maximum(events_t events) {

    if (events.empty()) {
      return {};
    }

    double max = metric_to_double(events[0]);

    for (const auto & e: events) {

      auto tmp = metric_to_double(e);
      if (tmp > max) {
        max = tmp;
      }
    }

    return max;
}
