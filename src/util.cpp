#include "util.h"
#include <string>
#include <sstream>

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
