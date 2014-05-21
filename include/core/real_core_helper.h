#ifndef CORE_REAL_CORE_HELPER_H
#define CORE_REAL_CORE_HELPER_H

#include <functional>
#include <core/real_core.h>

void detach_thread(std::function<void()> fn);

std::shared_ptr<core_interface> make_real_core(const config conf);

std::unique_ptr<riemann_tcp_pool> init_tcp_server(
    const config & conf,
    main_async_loop_interface & loop,
    std::shared_ptr<streams> streams);

std::unique_ptr<riemann_udp_pool> init_udp_server(
    const config & conf,
    std::shared_ptr<streams> streams);

std::unique_ptr<websocket_pool> init_ws_server(
    const config & conf,
    main_async_loop_interface & loop,
    pub_sub & pubsub);

#endif
