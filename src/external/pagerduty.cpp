#include <glog/logging.h>
#include <thread>
#include <pagerduty.h>
#include <util.h>
#include <core/core.h>

streams_t pd_trigger(const std::string & pg_key) {

  return create_stream(
      [=](forward_fn_t, const Event & event)
      {
        g_core->externals()->pager_duty_trigger(pg_key, event);
      }
  );

}

streams_t pd_resolve(const std::string & pg_key) {

 return create_stream(
      [=](forward_fn_t, const Event & event)
      {
        g_core->externals()->pager_duty_resolve(pg_key, event);
      }
  );

}

streams_t pd_acknowledge(const std::string & pg_key) {

 return create_stream(
      [=](forward_fn_t, const Event & event)
      {
        g_core->externals()->pager_duty_acknowledge(pg_key, event);
      }
  );

}
