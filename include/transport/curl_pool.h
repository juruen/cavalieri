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
    typedef std::function<void(const long)> set_timer_cb_t;
    typedef std::function<void(const int, const int,
                               const bool)>            multi_socket_cb_t;

  private:
    void on_create(int fd, async_loop & loop, tcp_connection & conn);
    void on_ready(async_fd & async, tcp_connection & conn);
    void async(async_loop & loop);
    void timer(async_loop & loop);

    void set_fd(const size_t loop_id, const int fd, async_fd::mode mode);
    void remove_fd(const size_t loop_id, const int fd);

    void check_multi_info(const size_t loop_id);
    void multi_timer(const size_t loop_id, const long ms);
    void multi_socket_action(const size_t loop_id);
    void multi_socket(const size_t loop_id, const int fd,
                      const int mode, const bool initialized);

  private:
    typedef tbb::concurrent_bounded_queue<queued_event_t> event_queue_t;
    typedef std::queue<std::vector<char>> fd_event_queue_t;
    typedef std::pair<size_t, curl_pool &> sock_cb_info_t;
    typedef std::shared_ptr<create_socket_cb_t> c_sock_t;
    typedef std::function<void()> clean_up_fn_t;
    typedef std::pair<std::shared_ptr<CURL>,
                      std::pair<c_sock_t, clean_up_fn_t>>    curl_conn_pair_t;


    tcp_pool tcp_pool_;
    curl_event_fn_t curl_event_fn_;

    std::vector<std::shared_ptr<event_queue_t>> thread_event_queues_;

    std::vector<std::vector<curl_conn_pair_t>> curl_conns_;
    std::vector<std::unordered_map<int, std::shared_ptr<CURL>>> fd_curl_conns_;
    std::vector<std::unordered_map<int, c_sock_t>> fd_create_socket_cbs_;
    std::vector<std::unordered_set<int>> finished_fds_;
    std::vector<std::unordered_map<int, int>> fd_initial_modes_;

    size_t next_thread_;
    std::shared_ptr<bool> sock_inited_;

    std::vector<std::shared_ptr<CURLM>> curl_multis_;

    std::vector<std::unique_ptr<set_timer_cb_t>> set_timer_cbs_;
    std::vector<std::unique_ptr<multi_socket_cb_t>> multi_socket_cbs_;

};

#endif
