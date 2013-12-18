#include <glog/logging.h>
#include <websocket_pool.h>
#include <websocket.h>
#include <util.h>

namespace {
  const float check_queue_interval_ms = .200;
}

websocket_pool::websocket_pool(size_t thread_num, pub_sub & pubsub)
:
  tcp_pool_(thread_num),
  timers_(thread_num)
{
  using namespace std::placeholders;

  for (size_t i = 0; i < thread_num; i++) {
    event_queues_.push_back(std::make_shared<event_queue_t>());
  }
  for (auto event_queue: event_queues_) {
    all_events_fn_ = pubsub.subscribe("index", event_queue);
  }
  tcp_pool_.set_tcp_conn_hook(
      [&](int fd)
        {
          return std::make_shared<ws_connection>(
                      fd, new ws_util(), all_events_fn_);
        }
  );
  tcp_pool_.set_run_hook(std::bind(&websocket_pool::run_hook, this, _1, _2));
  tcp_pool_.start_threads();
}

void websocket_pool::add_client(int fd) {
  tcp_pool_.add_client(fd);
}

void websocket_pool::notify_event(const Event & event) {
  for (auto & queue: event_queues_) {
    queue->push(event);
  }
}

void websocket_pool::timer_callback(ev::timer & timer, int revents) {
  UNUSED_VAR(revents);

  size_t tid = tcp_pool_.ev_to_tid(timer);
  auto event_queue = event_queues_[tid];

  if (event_queue->empty()) {
    return;
  }

  auto conn_map = tcp_pool_.tcp_connection_map(tid);
  while (!event_queue->empty()) {
    Event event;
    if (!event_queue->try_pop(event)) {
      continue;
    }
    std::string ev_str(event_to_json(event));
    for (auto & pair : conn_map) {
      dynamic_cast<ws_connection*>(pair.second.get())->send_frame(ev_str);
    }
  }
}

void websocket_pool::run_hook(size_t tid, ev::dynamic_loop & loop) {
  timers_[tid].set(loop);
  timers_[tid].set<websocket_pool, &websocket_pool::timer_callback>(this);
  timers_[tid].start(0, check_queue_interval_ms);
}
