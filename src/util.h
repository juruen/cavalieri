#ifndef UTIL_H
#define UTIL_H

#include "proto.pb.h"
#include <ev++.h>
#include <functional>

class CallbackTimer {
  private:
    ev::timer tio_;
    std::function<void()> f_;
  public:
    CallbackTimer(const int interval, const std::function<void()> f);
    void callback();
};

std::string metric_to_string(const Event& e);

#endif

