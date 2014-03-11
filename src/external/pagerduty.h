#ifndef PAGERDUTY_H
#define PAGERDUTY_H

#include <xtreams.h>

xtream_node_t pd_trigger(const std::string & pg_key);
xtream_node_t pd_resolve(const std::string & pg_key);
xtream_node_t pd_acknowledge(const std::string & pg_key);

#endif
