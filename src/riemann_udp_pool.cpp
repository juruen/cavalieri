#include <riemann_udp_pool.h>

riemann_udp_pool::riemann_udp_pool(raw_msg_fn_t raw_msg_fn)
  :
    udp_pool_(1, raw_msg_fn)
{
  udp_pool_.start_threads();
}

riemann_udp_pool::~riemann_udp_pool() {}
