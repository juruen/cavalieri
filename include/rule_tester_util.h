#ifndef CAVALIERI_RULE_TESTER_UTIL_H
#define CAVALIERI_RULE_TESTER_UTIL_H

#include <vector>
#include <common/event.h>
#include <external/mock_external.h>

typedef std::pair<time_t, Event> mock_index_events_t;

std::vector<Event> json_to_events(const std::string json, bool & ok);

std::string results(std::vector<mock_index_events_t> index_events,
                    std::vector<external_event_t> external_events);

#endif
