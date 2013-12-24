#ifndef PAGERDUTY_H
#define PAGERDUTY_H

#include <streams.h>

stream_t pd_trigger(const std::string& pg_key);
stream_t pd_resolve(const std::string& pg_key);
stream_t pd_acknowledge(const std::string& pg_key);

#endif
