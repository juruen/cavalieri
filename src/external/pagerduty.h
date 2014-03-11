#ifndef PAGERDUTY_H
#define PAGERDUTY_H

#include <xtreams.h>

xtreams_t pd_trigger(const std::string & pg_key);
xtreams_t pd_resolve(const std::string & pg_key);
xtreams_t pd_acknowledge(const std::string & pg_key);

#endif
