#include <folds.h>
#include <util.h>

mstream_t fold(const fold_fn_t f, const children_t & children) {
  return [=](const  events_t & events) {
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
