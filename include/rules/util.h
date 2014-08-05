#ifndef RULES_UTIL_H
#define RULES_UTIL_H

#include <streams/stream_functions.h>

streams_t max_critical_hosts(size_t n);

struct target_t {
  streams_t pagerduty;
  streams_t email;
  streams_t index;
  streams_t all;
};

target_t create_targets(const std::string pd_key, const std::string email);

#endif
