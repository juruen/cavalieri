#ifndef TCP_CLIENT_POOL_H
#define TCP_CLIENT_POOL_H

#include <tbb/concurrent_queue.h>
#include <functional>
#include <queue>
#include <unordered_map>
#include <transport/tcp_pool.h>
#include <transport/tcp_connection.h>
#include <proto.pb.h>

/* This callback is used to translate events into whatever needs to be
 * sent in the wire.
 */
typedef std::function<std::vector<char>(const Event & event)> output_event_fn_t;

class tcp_client_pool {
  public:
    tcp_client_pool(size_t thread_num, output_event_fn_t output_event_fn );
    void add_client (int fd);
    void push_event(const Event & event);

  private:
    void create_conn(int fd, async_loop & loop, tcp_connection & conn);
    void data_ready(async_fd & async, tcp_connection & conn);
    void async(async_loop & loop);

  private:
    typedef tbb::concurrent_bounded_queue<Event> event_queue_t;
    typedef std::queue<std::vector<char>> fd_event_queue_t;
    typedef std::pair<tcp_connection &, fd_event_queue_t> fd_conn_data_t;

    tcp_pool tcp_pool_;
    std::vector<std::shared_ptr<event_queue_t>> thread_event_queues_;
    std::vector<std::unordered_map<int, fd_conn_data_t>> fd_event_queues_;
    output_event_fn_t output_event_fn_;
};

#endif
