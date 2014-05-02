#include <glog/logging.h>
#include <thread>
#include <pagerduty.h>
#include <util.h>
#include <external/pagerduty_pool.h>

auto pd_pool = std::make_shared<pagerduty_pool>(1);

static streams_t call_pg(const std::string pg_key,
                         const pagerduty_pool::pd_action action)
{
  return create_stream(
    [=](forward_fn_t, const Event & event) {

      pd_pool->push_event(action, pg_key, event);

  });
}

streams_t pd_trigger(const std::string & pg_key) {
  return call_pg(pg_key, pagerduty_pool::pd_action::trigger);
}

streams_t pd_resolve(const std::string & pg_key) {
  return call_pg(pg_key, pagerduty_pool::pd_action::resolve);
}

streams_t pd_acknowledge(const std::string & pg_key) {
  return call_pg(pg_key, pagerduty_pool::pd_action::acknowledge);
}
