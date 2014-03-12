#ifndef PAGERDUTY_H
#define PAGERDUTY_H

#include <streams.h>

streams_t pd_trigger(const std::string & pg_key);
streams_t pd_resolve(const std::string & pg_key);
streams_t pd_acknowledge(const std::string & pg_key);

#endif
