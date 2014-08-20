#ifndef CAVALIERI_WEBSOCKET_COMMON_H
#define CAVALIERI_WEBSOCKET_COMMON_H

#include <functional>
#include <vector>
#include <proto.pb.h>

using event_filters_t = std::vector<std::function<void(const Event &)>>;

const size_t k_max_ws_queue_size = 20E4;

#endif
