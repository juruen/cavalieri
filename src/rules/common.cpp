#include "common.h"
#include <atomic>

namespace {
stream_t set_state(std::string state, children_t children) {
  return with({{"state", state}}, children);
}

stream_t  call_rescue_e(children_t children) {
  return [=](e_t e) { call_rescue(e, children);};
}

}

stream_t critical_above(double value, children_t children) {
  return
    split(
    {
      {above_eq_pred(value), set_state("critical", children)},
      {under_pred(value), set_state("ok", children)}
    });
}

stream_t critical_under(double value, children_t children) {
  return
    split(
    {
      {under_eq_pred(value), set_state("critical", children)},
      {above_eq_pred(value), set_state("ok", children)}
    });
}

typedef std::shared_ptr<std::atomic<bool>> atomic_bool_t;

stream_t stable_stream(double dt, atomic_bool_t state, children_t children) {
  auto set_state = [=](e_t e) {
    state->store(e.state() == "ok");
    call_rescue(e, children);
  };
  return (stable(dt, {set_state}));
}

stream_t trigger_detrigger(double dt, predicate_t trigger_pred,
                           predicate_t cancel_pred, children_t children)
{
  auto state_ok = std::make_shared<std::atomic<bool>>(true);
  auto s = stable_stream(dt, state_ok, {call_rescue_e(children)});

  return [=](e_t e) {
    bool ok = state_ok->load();
    std::string state("ok");
    if (trigger_pred(e) || (!ok && cancel_pred(e))) {
      state = "critical";
    }
    Event ne(e);
    ne.set_state(state);
    s(ne);
  };
}

stream_t trigger_detrigger_above(double dt, double trigger_value,
                                 double keep_trigger_value,
                                 children_t children)
{
  return trigger_detrigger(
      dt,
      above_pred(trigger_value),
      above_pred(keep_trigger_value),
      children
  );
}

stream_t trigger_detrigger_under(double dt, double trigger_value,
                                 double keep_trigger_value,
                                 children_t children)
{
  return trigger_detrigger(
      dt,
      under_pred(trigger_value),
      under_pred(keep_trigger_value),
      children
  );
}


