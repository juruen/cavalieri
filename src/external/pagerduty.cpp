#include <glog/logging.h>
#include <thread>
#include <pagerduty.h>
#include <util.h>
#include <python_interpreter.h>

static xtreams_t call_pg( const std::string& pg_key, const std::string& action)
{
  return create_xtream_node(
    [=](forward_fn_t, const Event & event) {

      Event me(event);

      Attribute* attr = me.add_attributes();
      attr->set_key("pg_key");
      attr->set_value(pg_key);

      const std::string jsonstr = event_to_json(me);

      VLOG(3) << "call_pg() pg_key: " << pg_key << " action: " << action
        << " event: " << jsonstr;

      g_python_runner.run_function("pagerduty", action, event_to_json(me));

  });
}

xtreams_t pd_trigger(const std::string& pg_key) {
  return call_pg(pg_key, "trigger");
}

xtreams_t pd_resolve(const std::string& pg_key) {
  return call_pg(pg_key, "resolve");
}

xtreams_t pd_acknowledge(const std::string& pg_key) {
  return call_pg(pg_key, "acknowledge");
}
