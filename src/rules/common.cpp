#include <atomic>
#include <folds.h>
#include <streams/stream_functions.h>
#include <rules/common.h>


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

streams_t trigger_detrigger(double dt, predicate_t trigger_pred,
                           predicate_t cancel_pred)
{
  auto state_ok = std::make_shared<std::atomic<bool>>(true);

  auto is_critical = [=](e_t e) {

    if (trigger_pred(e)) {
      return true;
    }

    if (!state_ok->load() && cancel_pred(e)) {
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

streams_t trigger_detrigger_above(double dt, double trigger_value,
                                  double keep_trigger_value)
{
  return trigger_detrigger(dt, above_pred(trigger_value),
                           above_pred(keep_trigger_value));
}

streams_t trigger_detrigger_under(double dt, double trigger_value,
                                 double keep_trigger_value)
{
  return trigger_detrigger(dt, under_pred(trigger_value),
                           under_pred(keep_trigger_value));
}

streams_t agg_trigger(double dt, fold_fn_t agg_fn, predicate_t trigger_pred,
                      predicate_t keep_trigger_pred)
{
  return
    coalesce(agg_fn)
      >> trigger_detrigger(dt, trigger_pred, keep_trigger_pred);
}

streams_t agg_sum_trigger_above(double dt, double trigger_value,
                               double keep_trigger_value)
{
  return agg_trigger(dt, sum, above_eq_pred(trigger_value),
                     above_pred(keep_trigger_value));
}

