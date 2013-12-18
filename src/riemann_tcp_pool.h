#ifndef RIEMANN_TCP_POOL
#define RIEMANN_TCP_POOL

#include <tcp_pool.h>
#include <incomingevents.h>

class riemann_tcp_pool {
  public:
    riemann_tcp_pool(size_t thread_num, incoming_events & ievents);
    void add_client (int fd);

  private:
    tcp_pool tcp_pool_;
};

#endif
