#ifndef CAVALIERI_FOLDS_H
#define CAVALIERI_FOLDS_H

#include <common/event.h>

Event sum(const std::vector<Event> & events);

Event product(const std::vector<Event> & events);

Event difference(const std::vector<Event> & events);

Event mean(const std::vector<Event> & events);

Event minimum(const std::vector<Event> & events);

Event maximum(const std::vector<Event> &  events);

Event count(const std::vector<Event> & events);

#endif
