#ifndef RIEMANN_UDP_POOL
#define RIEMANN_UDP_POOL

#include <transport/udp_pool.h>

typedef std::function<void(const std::vector<unsigned char>)> raw_msg_fn_t;

class riemann_udp_pool {
  public:
    riemann_udp_pool(raw_msg_fn_t raw_msg_fn);
    ~riemann_udp_pool();

  private:
    udp_pool udp_pool_;
};

#endif
