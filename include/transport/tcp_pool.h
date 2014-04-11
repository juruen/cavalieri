#ifndef TRANSPORT_TCP_POOL_H
#define TRANSPORT_TCP_POOL_H

#include <map>
#include <queue>
#include <mutex>
#include <pool/thread_pool.h>

class tcp_connection;

typedef std::map<const int, tcp_connection> conn_map_t;

typedef std::function<void(int fd,
                           async_loop & loop,
                           tcp_connection &)> tcp_create_conn_fn_t;

typedef std::function<void(async_fd &, tcp_connection&)> tcp_ready_fn_t;

class tcp_pool {
  public:
    tcp_pool(
        size_t thread_num,
        hook_fn_t run_fn,
        tcp_create_conn_fn_t create_conn_fn,
        tcp_ready_fn_t tcp_ready_fn_t
    );
    tcp_pool(
        size_t thread_num,
        hook_fn_t run_fn,
        tcp_create_conn_fn_t create_conn_fn,
        tcp_ready_fn_t tcp_ready_fn_t,
        const float inteval,
        timer_cb_fn_t timer_cb_fn
    );
    void add_client(int fd);
    void start_threads();
    void stop_threads();
    virtual ~tcp_pool();

  private:
    void async_hook(async_loop & loop);
    void socket_callback(async_fd & async);

  private:
    thread_pool thread_pool_;
    std::vector<std::mutex> mutexes_;
    std::vector<std::queue<int>> new_fds_;
    std::vector<conn_map_t> conn_maps_;
    tcp_create_conn_fn_t tcp_create_conn_fn_;
    tcp_ready_fn_t tcp_ready_fn_;
};

#endif
