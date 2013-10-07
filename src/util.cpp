#include "util.h"
#include <string>
#include <sstream>
#include <glog/logging.h>

CallbackTimer::CallbackTimer(const int interval, const std::function<void()> f)
  : f_(f)
{
      tio_.set<CallbackTimer, &CallbackTimer::callback>(this);
      tio_.start(0, interval);
}

void CallbackTimer::callback() {
  f_();
}

std::string metric_to_string(const Event& e) {
  std::ostringstream ss;
  if (e.has_metric_f()) {
    ss << e.metric_f();
  } else if (e.has_metric_d()) {
    ss << e.metric_d();
  } else if (e.has_metric_sint64()) {
    ss << e.metric_sint64();
  } else {
    ss << "";
  }
  return ss.str();
}

double metric_to_double(const Event &e) {
  if (e.has_metric_f()) {
    return e.metric_f();
  } else if (e.has_metric_sint64()) {
    return e.metric_sint64();
  } else if (e.has_metric_d()) {
    return e.metric_d();
  } else {
    return 0;
  }
}

std::string string_to_value(const Event& e, const std::string& key) {
  if (key == "host") {
    return e.host();
  } else if (key == "service") {
    return e.service();
  } else if (key == "description") {
    return e.description();
  } else if (key == "state") {
    return e.state();
  } else if (key == "metric") {
    return metric_to_string(e);
  } else {
    return "__nil__";
  }
}


void set_event_value(Event& e, const std::string& key, const std::string& value) {
  if (key == "host") {
    e.set_host(value);
  } else if (key == "service") {
    e.set_service(value);
  } else if (key == "description") {
    e.set_description(value);
  } else if (key == "state") {
    e.set_state(value);
  } else if (key == "metric") {
    if (e.has_metric_f()) {
      e.set_metric_f(atof(value.c_str()));
    } else if (e.has_metric_d()) {
      e.set_metric_d(atof(value.c_str()));
    } else if (e.has_metric_sint64()) {
      e.set_metric_sint64(atoi(value.c_str()));
    }
  } else {
    LOG(ERROR) << "string_to_value() wrong key: " << key;
  }
}

bool tag_exists(const Event& e, const std::string& tag) {
  for (int i = 0; i < e.tags_size(); i++) {
    if (e.tags(i) == tag) {
      return true;
    }
  }

  return false;
}
