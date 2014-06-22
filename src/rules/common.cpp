#include <atomic>
#include <folds.h>
#include <util.h>
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

bool compare_event_time(const Event & a, const Event & b) {

  return a.time() < b.time();

}

const int64_t k_same_interval = 60;

bool same_interval(const std::vector<Event> events) {

  auto min = std::min_element(begin(events), end(events), compare_event_time);
  auto max = std::max_element(begin(events), end(events), compare_event_time);

  return (max->time() - min->time()) < k_same_interval;
}

fold_fn_t fold_ratio(const double zero_ratio) {

  return [=](const std::vector<Event> events)
  {
    if (events.size() == 2 && metric_set(events[0]) && metric_set(events[1])
        && same_interval(events))
    {

      auto a = metric_to_double(events[0]);
      auto b = metric_to_double(events[1]);

      double ratio;
      if (b == 0) {
        ratio = zero_ratio;
      } else {
        ratio = a / b;
      }

      Event e(events[0]);
      e.set_service(events[0].service() + " / " + events[1].service());

      return set_metric(e, ratio);

    } else {
      return Event();
    }
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

streams_t ratio(const std::string a, const std::string b,
                const double default_zero)
{
  return project({service_pred(a), service_pred(b)}, fold_ratio(default_zero))
         >> where(PRED(metric_set(e)));
}

streams_t per_host_ratio(const std::string a, const std::string b,
                         const double default_zero, double dt,
                         predicate_t trigger,
                         predicate_t cancel)
{
  return by({"host"}, BY(ratio(a, b, default_zero)))
         >> stable_metric(dt, trigger, cancel);
}

target_t create_targets(const std::string pagerduty_key, const std::string to) {
  target_t target;

  auto pg_stream = sdo(state("ok")       >> pagerduty_resolve(pagerduty_key),
                       state("critical") >> pagerduty_trigger(pagerduty_key));

  auto mail_stream = email("localhost", "cavalieri@localhost", to);

  target.pagerduty = changed_state("ok") >> pg_stream;
  target.email = changed_state("ok") >> mail_stream;
  target.index = send_index();
  target.all = sdo(target.pagerduty, target.email, target.index);

  return target;
}
