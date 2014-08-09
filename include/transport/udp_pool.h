#ifndef CAVALIERI_TRANSPORT_UDP_POOL_H
#define CAVALIERI_TRANSPORT_UDP_POOL_H

#include <vector>
#include <pool/async_thread_pool.h>

typedef std::vector<unsigned char> udp_buffer_t;
typedef std::function<void(const udp_buffer_t &)> udp_read_fn_t;

class udp_pool {
  public:
    udp_pool(
        size_t thread_num,
        uint32_t port,
        udp_read_fn_t udp_ready_fn_t
    );
    void start_threads();
    void stop_threads();
    virtual ~udp_pool();

  private:
    void run_hook(async_loop & loop);
    void socket_callback(async_fd & async);

  private:
    async_thread_pool async_thread_pool_;
    uint32_t port_;
    udp_read_fn_t udp_read_fn_;
};

#endif
