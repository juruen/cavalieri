#include <glog/logging.h>
#include <thread>
#include <pagerduty.h>
#include <util.h>
#include <pythoninterpreter.h>


static stream_t call_pg( const std::string& pg_key, const std::string& action)
{
  return [=](e_t event) {
    std::thread py_thread([=](){
        Event me(event);
        Attribute* attr = me.add_attributes();
        attr->set_key("pg_key");
        attr->set_value(pg_key);
        python_interpreter python;
        const std::string jsonstr = event_to_json(me);
        VLOG(3) << "call_pg() pg_key: " << pg_key << " action: " << action
                << " event: " << jsonstr;
        python.run_function("pagerduty", action, event_to_json(me));
    });
    py_thread.detach();
  };
}

stream_t pd_trigger(const std::string& pg_key) {
  return call_pg(pg_key, "trigger");
}

stream_t pd_resolve(const std::string& pg_key) {
  return call_pg(pg_key, "resolve");
}

stream_t pd_acknowledge(const std::string& pg_key) {
  return call_pg(pg_key, "acknowledge");
}


