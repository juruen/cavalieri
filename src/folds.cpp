#include <folds.h>
#include <util.h>

mstream_t fold(const fold_fn_t f, const children_t & children) {
  return [=](const  events_t & events) {
    //TODO: Filter nil metric events
    if (events.empty()) {
      return;
    }

    Event event(events[0]);

    double t(metric_to_double(event));
    for (size_t i = 1; i < events.size(); i++) {
      t = f(t, metric_to_double(events[i]));
    }
    clear_metrics(event);
    event.set_metric_d(t);

    call_rescue(event, children);
  };
}


mstream_t sum(const children_t children) {
  auto sum_fn = [](const double & x, const double & y) { return (x + y);};
  return fold(sum_fn, children);
}

mstream_t product(const children_t children) {
  auto product_fn = [](const double & x, const double & y) { return (x * y); };
  return fold(product_fn, children);
}

mstream_t difference(const children_t children) {
  auto difference_fn = [](const double & x, const double & y) { return (x - y);};
  return fold(difference_fn, children);
}
