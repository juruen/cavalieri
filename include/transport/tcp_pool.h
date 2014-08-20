#ifndef CAVALIERI_TRANSPORT_TCP_POOL_H
#define CAVALIERI_TRANSPORT_TCP_POOL_H

#include <map>
#include <queue>
#include <mutex>
#include <pool/async_thread_pool.h>

class tcp_connection;

/* This callback is run when a thread adds the file descriptor that
 * is passed from add_client() to its event loop.
 *
 * This is executed in the context of the thread that owns the file descriptor.
 *
 * You can use the passed tcp_connection to write and read from the socket
 * using the provided buffers in it.
 */
typedef std::function<void(int fd, async_loop &,
                           tcp_connection &)> tcp_create_conn_fn_t;


/* This callback is run when a file descriptor is ready to read or write.
 *
 * This is executed in the context of the thread that owns the file descriptor.
 *
 */
typedef std::function<void(async_fd &, tcp_connection &)> tcp_ready_fn_t;

/* This callback is run when the thread's loop is signaled.
 *
 * This is executed in the context of the thread that owns the loop.
 *
 */
typedef std::function<void(async_loop &)> async_fn_t;

class tcp_pool {
  public:
    tcp_pool(
        size_t thread_num,
        hook_fn_t run_fn,
        tcp_create_conn_fn_t create_conn_fn,
        tcp_ready_fn_t tcp_ready_fn
    );
    tcp_pool(
        size_t thread_num,
        hook_fn_t run_fn,
        tcp_create_conn_fn_t create_conn_fn,
        tcp_ready_fn_t tcp_ready_fn_t,
        const float interval,
        timer_cb_fn_t timer_cb_fn
    );
    tcp_pool(
        size_t thread_num,
        hook_fn_t run_fn,
        tcp_create_conn_fn_t create_conn_fn,
        tcp_ready_fn_t tcp_ready_fn_t,
        const float interval,
        timer_cb_fn_t timer_cb_fn,
        async_fn_t async_fn
    );

    /* Async. Can be used from any thread */
    void add_client(const int fd);
    void add_client(const size_t loop_id, int fd);

    /* Sync. Must be used for the thread that owns loop_id */
    void add_client_sync(const size_t loop_id, int fd);
    void remove_client_sync(const size_t loop_id, int fd);

    void signal_thread(const size_t loop_id);
    void signal_threads();
    void start_threads();
    void stop_threads();
    async_loop & loop(const size_t loop_id);
    virtual ~tcp_pool();

  private:
    void async_hook(async_loop & loop);
    void add_fds(async_loop & loop);
    void socket_callback(async_fd & async);

  private:
    typedef std::map<int, tcp_connection> conn_map_t;

    async_thread_pool async_thread_pool_;
    std::vector<std::mutex> mutexes_;
    std::vector<std::queue<int>> new_fds_;
    std::vector<conn_map_t> conn_maps_;
    tcp_create_conn_fn_t tcp_create_conn_fn_;
    tcp_ready_fn_t tcp_ready_fn_;
    async_fn_t async_fn_;
};

#endif
