#ifndef WEBSOCKET_POOL
#define WEBSOCKET_POOL

#include <tbb/concurrent_queue.h>
#include <functional>
#include <queue>
#include <transport/tcp_pool.h>
#include <transport/ws_connection.h>
#include <websocket_pool.h>
#include <pub_sub/pub_sub.h>

class websocket_pool {
  public:
    websocket_pool(size_t thread_num, pub_sub & pubsub);
    void add_client (int fd);
    void notify_event(const Event& event);
    void stop();

  private:
    void create_conn(int fd, async_loop & loop, tcp_connection & conn);
    void data_ready(async_fd & async, tcp_connection & conn);
    void timer(async_loop & loop);

  private:
    typedef tbb::concurrent_bounded_queue<Event> event_queue_t;
    typedef std::tuple<ws_connection,
                       std::function<bool(const Event&)>,
                       std::queue<std::string>> fd_conn_data_t;

    tcp_pool tcp_pool_;
    std::vector<std::shared_ptr<event_queue_t>> thread_event_queues_;
    std::vector<std::map<int, fd_conn_data_t>> fd_event_queues_;
    all_events_fn_t all_events_fn_;
};

#endif
