#ifndef WEBSOCKET_POOL
#define WEBSOCKET_POOL

#include <tbb/concurrent_queue.h>
#include <tcp_pool.h>
#include <pubsub.h>

typedef tbb::concurrent_queue<Event> event_queue_t;

class websocket_pool {
  public:
    websocket_pool(size_t thread_num, pub_sub & pubsub);
    void add_client (int fd);
    void notify_event(const Event& event);
    void timer_callback(ev::timer & timer, int revents);
    void run_hook(size_t tid, ev::dynamic_loop & loop);

  private:
    tcp_pool tcp_pool_;
    std::vector<std::shared_ptr<event_queue_t>> event_queues_;
    std::vector<ev::timer> timers_;
    allevents_f_t all_events_fn_;

};

#endif
