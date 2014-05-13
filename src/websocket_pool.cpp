#include <glog/logging.h>
#include <websocket_pool.h>
#include <transport/ws_connection.h>
#include <util.h>
#include <iostream>
#include <driver.h>
#include <expression.h>


namespace {
  const size_t k_queue_capacity = 1000000;
  const float  k_ws_send_interval = 1;

  std::function<bool(const Event&)> filter_query(const std::string uri) {

    std::string index;
    std::map<std::string, std::string> params;

    query_fn_t true_query = [](const Event&) -> bool { return true; };

    if (!parse_uri(uri, index, params)) {
      VLOG(1) << "failed to parse uri";
      return true_query;
    }

    query_context query_ctx;
    queryparser::driver driver(query_ctx);

    if (driver.parse_string(params["query"], "query")) {
      return  query_ctx.evaluate();
    } else {
      return true_query;
    }
  }

  async_fd::mode conn_to_mode(const tcp_connection & conn) {
    if (conn.pending_read() && conn.pending_write()) {
      return async_fd::readwrite;
    } else if (conn.pending_read()) {
      return async_fd::read;
    } else if (conn.pending_write()) {
      return async_fd::write;
    } else {
      return async_fd::none;
    }
  }

}

using namespace std::placeholders;

websocket_pool::websocket_pool(size_t thread_num, pub_sub & pubsub)
:
  tcp_pool_(thread_num,
            {},
            std::bind(&websocket_pool::create_conn, this, _1, _2, _3),
            std::bind(&websocket_pool::data_ready, this, _1, _2),
            k_ws_send_interval,
            std::bind(&websocket_pool::timer, this, _1)),
  thread_event_queues_(0),
  fd_event_queues_(thread_num)
{

  for (size_t i = 0; i < thread_num; i++) {

    auto queue = std::make_shared<event_queue_t>();
    queue->set_capacity(k_queue_capacity);

    thread_event_queues_.push_back(queue);

    all_events_fn_ = pubsub.subscribe("index", queue);

  }

  tcp_pool_.start_threads();
}

void websocket_pool::add_client(int fd) {
  tcp_pool_.add_client(fd);
}

void websocket_pool::notify_event(const Event & event) {
  for (auto & queue: thread_event_queues_) {
    queue->push(event);
  }
}

void websocket_pool::create_conn(int fd, async_loop & loop,
                                 tcp_connection & conn)
{
  auto conn_data = std::make_tuple(ws_connection(conn),
                           std::function<bool(const Event&)>{},
                           std::queue<std::string>{});

  fd_event_queues_[loop.id()].insert({fd, std::move(conn_data)});
}

void websocket_pool::data_ready(async_fd & async, tcp_connection & tcp_conn) {

  auto & fd_conn = fd_event_queues_[async.loop().id()];

  auto it = fd_conn.find(async.fd());
  CHECK(it != fd_conn.end()) << "couldn't find fd";

  auto & ws_conn = std::get<0>(it->second);
  ws_conn.callback(async);

  async.set_mode(conn_to_mode(tcp_conn));

  if (tcp_conn.close_connection) {
    VLOG(1) << "Closing websocket connection";
    fd_conn.erase(it);
    return;
  }

  // Set query_fn if websocket is ready
  auto & query_fn = std::get<1>(it->second);
  if (!query_fn && (ws_conn.state() | ws_connection::k_read_frame_header)) {

    query_fn = filter_query(ws_conn.uri());

    auto & str_queue = std::get<2>(it->second);
    for (const auto & event : all_events_fn_()) {

      if (query_fn(event)) {
        str_queue.push(event_to_json(event));
      }
    }

  }
}

void websocket_pool::timer(async_loop & loop) {
  size_t loop_id =  loop.id();
  VLOG(3) << "loop_id: " << loop_id;
  auto event_queue = thread_event_queues_[loop_id];

  while (!event_queue->empty()) {

    Event event;
    if (!event_queue->try_pop(event)) {
      continue;
    }

    auto str_event = event_to_json(event);

    for (auto & fd_conn : fd_event_queues_[loop_id]) {

      auto query_fn = std::get<1>(fd_conn.second);

      if (!query_fn) {
        VLOG(3) << "query_fn not defined";
        continue;
      }

      if (!query_fn(event)) {
        continue;
      }

      auto ws_conn = std::get<0>(fd_conn.second);
      auto & str_queue = std::get<2>(fd_conn.second);

      if (str_queue.empty() && ws_conn.send_frame(str_event)) {
        loop.set_fd_mode(fd_conn.first, async_fd::readwrite);
        continue;
      }

      if (str_queue.size() > k_queue_capacity) {
        VLOG(1) << "Per connection event queue is full";
      } else {
        str_queue.push(str_event);
      }

    }
  }

  for (auto & fd_conn : fd_event_queues_[loop_id]) {

    auto ws_conn = std::get<0>(fd_conn.second);
    auto & str_queue = std::get<2>(fd_conn.second);

    while (!str_queue.empty()) {

      std::string str = str_queue.front();

      if (!ws_conn.send_frame(str)) {
        VLOG(1) << "buffer full to send frame";
        break;
      }

      str_queue.pop();

      loop.set_fd_mode(fd_conn.first, async_fd::readwrite);
    }
  }
}
