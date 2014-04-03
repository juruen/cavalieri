#ifndef FOLDS_H
#define FOLDS_H

#include <proto.pb.h>

Event sum(const std::vector<Event> events);

Event product(const std::vector<Event> events);

Event difference(const std::vector<Event> events);

Event mean(const std::vector<Event> events);

Event minimum(const std::vector<Event> events);

Event maximum(const std::vector<Event> events);

#endif
