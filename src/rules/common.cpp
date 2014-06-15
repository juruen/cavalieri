#include <atomic>
#include <folds.h>
#include <algorithm>
#include <streams/stream_functions.h>
#include <rules/common.h>

namespace {

typedef std::shared_ptr<std::atomic<bool>> atomic_bool_t;

streams_t stable_stream(double dt, atomic_bool_t state) {

  auto set_state = create_stream(
    [=](forward_fn_t forward, e_t e)
    {
      state->store(e.state() == "ok");
      forward(e);
    });

  return stable(dt) >> set_state;
}

fold_fn_t fold_max_critical_hosts(size_t n) {

  return [=](std::vector<Event> events)
  {
    auto c = std::count_if(begin(events), end(events),
                           [](e_t e) { return e.state() == "critical"; });


    Event e(events[0]);
    e.set_state(static_cast<size_t>(c) >= n ? "critical" : "ok");

    return e;
  };
}

}

streams_t critical_above(double value) {
  return
    split(
    {
      {above_eq_pred(value), set_state("critical")},
      {default_pred(), set_state("ok")}
    });
}

streams_t critical_under(double value) {
  return
    split(
    {
      {under_eq_pred(value), set_state("critical")},
      {default_pred(), set_state("ok")}
    });

}

streams_t stable_metric(double dt, predicate_t trigger)
{

  split_clauses_t clauses =
  {
    {trigger, set_state("critical")},
    {default_pred(), set_state("ok")}
  };

  return split(clauses) >>  stable(dt);

}

streams_t stable_metric(double dt, predicate_t trigger, predicate_t cancel)
{
  auto state_ok = std::make_shared<std::atomic<bool>>(true);

  auto is_critical = [=](e_t e) {

    if (trigger(e)) {
      return true;
    }

    if (!state_ok->load() && !cancel(e)) {
      return true;
    }

    return false;
  };

  split_clauses_t clauses =
  {
    {is_critical, set_state("critical")},
    {default_pred(), set_state("ok")}
  };

  return split(clauses) >>  stable_stream(dt, state_ok);
}



streams_t agg_stable_metric(double dt, fold_fn_t fold_fn, predicate_t trigger,
                            predicate_t cancel)
{
  return coalesce(fold_fn) >> stable_metric(dt, trigger, cancel);
}


streams_t max_critical_hosts(size_t n) {
  return coalesce(fold_max_critical_hosts(n));
}

