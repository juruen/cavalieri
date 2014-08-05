#ifndef RIEMANN_TCP_POOL
#define RIEMANN_TCP_POOL

#include <transport/tcp_pool.h>
#include <instrumentation/instrumentation.h>
#include <riemann_tcp_connection.h>

class riemann_tcp_pool {
  public:
    riemann_tcp_pool(size_t thread_num, raw_msg_fn_t raw_msg_fn,
                     instrumentation & instr);
    void add_client (int fd);
    void stop();
    ~riemann_tcp_pool();

  private:
    void create_conn(int fd, async_loop & loop, tcp_connection & conn);
    void data_ready(async_fd & async, tcp_connection & conn);

  private:
    tcp_pool tcp_pool_;
    raw_msg_fn_t raw_msg_fn_;
    instrumentation & instrumentation_;
    instrumentation::id_t gauge_id_;
    std::vector<std::map<int, riemann_tcp_connection>> connections_;
};

#endif
