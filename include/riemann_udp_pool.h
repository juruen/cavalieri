#ifndef CAVALIERI_RIEMANN_UDP_POOL
#define CAVALIERI_RIEMANN_UDP_POOL

#include <transport/udp_pool.h>

typedef std::function<void(const std::vector<unsigned char>)> raw_msg_fn_t;

class riemann_udp_pool {
  public:
    riemann_udp_pool(uint32_t port, raw_msg_fn_t raw_msg_fn);
    void stop();
    ~riemann_udp_pool();

  private:
    udp_pool udp_pool_;
};

#endif
