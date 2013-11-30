#ifndef TCP_POOL_H
#define TCP_POOL_H

#include <map>
#include <queue>
#include <thread_pool.h>

class tcp_connection;
typedef std::map<const int, std::shared_ptr<tcp_connection>> conn_map_t;
typedef std::function<std::shared_ptr<tcp_connection>(int fd)> tcp_conn_hook_t;

class tcp_pool {
  public:
    tcp_pool(size_t thread_num);
    void add_client(int fd);
    void set_tcp_conn_hook(tcp_conn_hook_t hook);
    void set_run_hook(hook_fn_t hook);
    void start_threads();
    void stop_threads();
    size_t ev_to_tid(ev::timer & timer);
    conn_map_t & tcp_connection_map(size_t tid);
    virtual ~tcp_pool();

  protected:
    void async_hook(size_t tid, ev::dynamic_loop & loop);
    void socket_callback(ev::io & io, int revents);
    std::shared_ptr<tcp_connection> create_tcp_connection(int fd);

  protected:
    thread_pool thread_pool_;
    size_t pool_size_;
    std::vector<std::mutex> mutexes_;
    std::vector<std::queue<int>> new_fds_;
    std::vector<conn_map_t> conn_maps_;
    tcp_conn_hook_t tcp_conn_hook_;
};

#endif
