#ifndef CAVALIERI_WEBSOCKET_WEBSOCKET_POOL_H
#define CAVALIERI_WEBSOCKET_WEBSOCKET_POOL_H

#include <functional>
#include <queue>
#include <websocket/worker_pool.h>
#include <transport/tcp_pool.h>
#include <transport/ws_connection.h>
#include <pub_sub/pub_sub.h>
#include <index/real_index.h>

class websocket_pool {
public:
  websocket_pool(size_t thread_num, pub_sub & pubsub, real_index & index);
  void add_client(const int fd);
  void stop();

private:
  using query_fn_t = std::function<bool(const Event&)>;
  using fd_queue_t = std::queue<std::string>;
  using fd_ctx_t = struct
                    {
                      ws_connection ws_cnx;
                      query_fn_t query_fn;
                      fd_queue_t queue;
                      size_t token;
                    };
  using event_t = struct
                    {
                      Event event;
                      int fd;
                      size_t token;
                    };

  using event_queue_t = tbb::concurrent_bounded_queue<event_t>;

private:
  void on_new_cnx(int fd, async_loop & loop, tcp_connection & cnx);
  void on_fd_ready(async_fd & async, tcp_connection & cnx);
  void on_async_signal(async_loop & loop);
  void on_timer(async_loop & loop);

  bool pending_ws_init(fd_ctx_t & fd_ctx);
  bool handle_ws_init(async_fd & async, fd_ctx_t & fd_ctx);

  bool flush_fd_queue(async_loop & loop, int fd, fd_ctx_t & fd_ctx);

  void update_filters(const size_t loop_id);

private:
  real_index & index_;
  tcp_pool tcp_pool_;
  worker_pool worker_pool_;
  std::vector<std::unordered_map<int, fd_ctx_t>> fd_ctxes_;
  std::vector<event_queue_t> event_queues_;
};

#endif
