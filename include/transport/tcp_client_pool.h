#ifndef CAVALIERI_TCP_CLIENT_POOL_H
#define CAVALIERI_TCP_CLIENT_POOL_H

#include <tbb/concurrent_queue.h>
#include <functional>
#include <queue>
#include <unordered_map>
#include <transport/tcp_pool.h>
#include <transport/tcp_connection.h>
#include <common/event.h>

/* This callback is used to translate events into whatever needs to be
 * sent in the wire.
 */
typedef std::function<std::vector<char>(const Event & event)>
        output_event_fn_t;

typedef std::function<std::vector<char>(const std::vector<Event>)>
        output_events_fn_t;


class tcp_client_pool {
  public:
    tcp_client_pool(size_t thread_num, const std::string host, const int port,
                    output_event_fn_t output_event_fn );
    tcp_client_pool(size_t thread_num, const std::string host, const int port,
                    size_t batch_size, output_events_fn_t output_events_fn);
    void push_event(const Event & event);
    void stop();

  private:
    void create_conn(int fd, async_loop & loop, tcp_connection & conn);
    void data_ready(async_fd & async, tcp_connection & conn);
    void async(async_loop & loop);
    void signal_batch_flush(const size_t loop_id);
    void connect_clients(const size_t loop_id);

  private:
    typedef tbb::concurrent_bounded_queue<Event> event_queue_t;
    typedef std::queue<std::vector<char>> fd_event_queue_t;
    typedef std::pair<tcp_connection &, fd_event_queue_t> fd_conn_data_t;

    tcp_pool tcp_pool_;
    const std::string host_;
    const int port_;
    std::vector<std::shared_ptr<event_queue_t>> thread_event_queues_;
    std::vector<std::unordered_map<int, fd_conn_data_t>> fd_event_queues_;
    output_event_fn_t output_event_fn_;
    output_events_fn_t output_events_fn_;
    bool batched_;
    size_t batch_size_;
    std::vector<int> flush_batch_;
    size_t next_thread_;

};

#endif
