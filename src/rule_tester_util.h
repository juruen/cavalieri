#ifndef RULE_TESTER_UTIL_H
#define RULE_TESTER_UTIL_H

#include <vector>
#include "proto.pb.h"

std::vector<Event> json_to_events(const std::string json, bool & ok);

#endif
