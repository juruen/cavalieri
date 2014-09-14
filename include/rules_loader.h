#ifndef CAVALIERI_RULES_LOADER_H
#define CAVALIERI_RULES_LOADER_H

#include <streams/stream_functions.h>

std::vector<std::shared_ptr<streams_t>> load_rules(const std::string file);

#endif
