#ifndef TRANSPORT_CURL_POOL_H
#define TRANSPORT_CURL_POOL_H

#include <tbb/concurrent_queue.h>
#include <functional>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <boost/optional.hpp>
#include <boost/any.hpp>
#include <curl/curl.h>
#include <proto.pb.h>
#include <transport/tcp_pool.h>


typedef struct {
  Event event;
  boost::any extra;
} queued_event_t;

typedef std::function<void (const queued_event_t,
                            const std::shared_ptr<CURL>,
                            std::function<void()> &)> curl_event_fn_t;

class curl_pool {
  public:
    curl_pool(const size_t thread_num, const curl_event_fn_t curl_event_fn);
    void push_event(const Event & event, const boost::any extra);

  public:
    typedef std::function<void(const int)> create_socket_cb_t;
    typedef std::function<void(const int)> close_socket_cb_t;
    typedef std::function<void(const long)> set_timer_cb_t;
    typedef std::function<void(const int, const int,
                               const bool)>            multi_socket_cb_t;

  private:
    void on_ready(async_fd & async, tcp_connection & conn);
    void async(async_loop & loop);
    void timer(async_loop & loop);

    void set_fd(const size_t loop_id, const int fd, async_fd::mode mode);

    void check_multi_info(const size_t loop_id);
    void multi_timer(const size_t loop_id, const long ms);
    void multi_socket_action(const size_t loop_id);
    void multi_socket(const size_t loop_id, const int fd,
                      const int mode, const bool initialized);

    void add_fd(size_t loop_id, CURL* curl_conn, int fd);
    void remove_fds(size_t loop_id, CURL* curl_conn);

    void cleanup_conns(const size_t loop_id);

  private:
    typedef tbb::concurrent_bounded_queue<queued_event_t> event_queue_t;

    typedef struct {
      std::shared_ptr<create_socket_cb_t> create_socket_fn;
      std::shared_ptr<close_socket_cb_t> close_socket_fn;
      std::function<void()> cleanup_fn;
      std::unordered_set<int> fds;
    } curl_conn_data_t;

    tcp_pool tcp_pool_;
    curl_event_fn_t curl_event_fn_;

    std::vector<std::shared_ptr<event_queue_t>> thread_event_queues_;

    std::vector<std::unordered_map<CURL*, curl_conn_data_t>> curl_conns_;
    std::vector<std::vector<CURL*>> finished_conns_;

    size_t next_thread_;

    std::vector<std::shared_ptr<CURLM>> curl_multis_;

    std::vector<std::unique_ptr<set_timer_cb_t>> set_timer_cbs_;
    std::vector<std::unique_ptr<multi_socket_cb_t>> multi_socket_cbs_;

};

#endif
