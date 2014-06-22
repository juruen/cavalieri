#ifndef RULES_COMMON_H
#define RULES_COMMON_H

#include <streams/stream_functions.h>

streams_t critical_above(double value);

streams_t critical_under(double value);

streams_t stable_metric(double dt, predicate_t trigger);

streams_t stable_metric(double dt, predicate_t trigger, predicate_t cancel);

streams_t agg_stable_metric(double dt, fold_fn_t fold_fn, predicate_t trigger,
                            predicate_t cancel);

streams_t max_critical_hosts(size_t n);

streams_t ratio(const std::string a, const std::string b,
                const double default_zero);

streams_t per_host_ratio(const std::string a, const std::string b,
                         const double default_zero, double dt,
                         predicate_t trigger,
                         predicate_t cancel);
struct target_t {
  streams_t pagerduty;
  streams_t email;
  streams_t index;
  streams_t all;
};

target_t create_targets(const std::string pd_key, const std::string email);

#endif
