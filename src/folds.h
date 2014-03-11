#ifndef FOLDS_H
#define FOLDS_H

#include <proto.pb.h>
#include <boost/optional.hpp>

typedef boost::optional<double> fold_result_t;

fold_result_t sum(const std::vector<Event> events);

fold_result_t product(const std::vector<Event> events);

fold_result_t difference(const std::vector<Event> events);

fold_result_t mean(const std::vector<Event> events);

fold_result_t minimum(const std::vector<Event> events);

fold_result_t maximum(const std::vector<Event> events);

#endif
