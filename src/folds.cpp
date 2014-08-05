#include <glog/logging.h>
#include <folds.h>
#include <util.h>
#include <functional>
#include <util.h>

namespace {

typedef std::function<double(double, double)> reduce_fn_t;
typedef std::vector<Event> events_t;
typedef std::function<Event(const events_t)> fold_fn_t;

double reduce(const reduce_fn_t & f, const events_t & events) {

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

double max_time(const events_t & events) {

  double max_time = 0;

  for (const auto & e : events) {
    if (e.time() > max_time) {
      max_time = e.time();
    }
  }

  return max_time;
}

Event fold(const reduce_fn_t & f, const events_t & events) {

    //TODO: Filter nil metric events
    if (events.empty()) {
      return {};
    }

    Event e(events.front());
    set_metric(e, reduce(f, events));
    e.set_time(max_time(events));

    return e;
}

}


Event sum(const events_t & events) {
  return fold(sum_fn, events);
}

Event product(const events_t & events) {
  return fold(product_fn, events);
}

Event difference(const events_t & events) {
  return fold(difference_fn, events);
}

Event mean(const events_t & events) {

  if (events.empty()) {
    return {};
  }

  Event e(events.front());
  e.set_metric_d(reduce(sum_fn, events) / events.size());
  e.set_time(max_time(events));

  return e;
}

Event minimum(const events_t & events) {

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

  Event e(events.front());
  e.set_metric_d(min);
  e.set_time(max_time(events));

  return e;
}

Event maximum(const events_t & events) {

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

  Event e(events.front());
  e.set_metric_d(max);
  e.set_time(max_time(events));

  return e;
}
