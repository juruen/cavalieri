#ifndef RULES_COMMON_H
#define RULES_COMMON_H

#include <streams.h>

stream_t critical_above(double value, children_t children);

stream_t critical_under(double value, children_t children);

stream_t trigger_detrigger_above(double dt, double trigger_value,
                                 double keep_trigger_value,
                                 children_t children);

stream_t trigger_detrigger_under(double dt, double trigger_value,
                                 double keep_trigger_value,
                                 children_t children);

stream_t agg_sum_trigger_above(double dt, double trigger_value,
                               double keep_trigger_value, children_t children);
#endif
