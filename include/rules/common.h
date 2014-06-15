#ifndef RULES_COMMON_H
#define RULES_COMMON_H

#include <streams/stream_functions.h>

streams_t critical_above(double value);

streams_t critical_under(double value);

streams_t stable_metric(double dt, predicate_t trigger);

streams_t stable_metric(double dt, predicate_t trigger, predicate_t cancel);

streams_t agg_stable_metric(double dt, fold_fn_t fold_fn, predicate_t trigger,
                            predicate_t cancel);

#endif
