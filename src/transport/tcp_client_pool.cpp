#include <glog/logging.h>
#include <functional>
#include <utility>
#include <transport/tcp_connection.h>
#include <transport/tcp_client_pool.h>

namespace {
  const size_t k_queue_capacity = 10000;
}

using namespace std::placeholders;

tcp_client_pool::tcp_client_pool(size_t thread_num,
                                 output_event_fn_t output_event_fn)
:
  tcp_pool_(
    thread_num,
    {},
    std::bind(&tcp_client_pool::create_conn, this, _1, _2, _3),
    std::bind(&tcp_client_pool::data_ready, this, _1, _2),
    std::bind(&tcp_client_pool::async, this, _1)
  ),
  thread_event_queues_(0),
  fd_event_queues_(thread_num),
  output_event_fn_(output_event_fn)
{

  for (size_t i = 0; i < thread_num; i++) {

    auto queue = std::make_shared<event_queue_t>();
    queue->set_capacity(k_queue_capacity);

    thread_event_queues_.push_back(queue);

  }

  tcp_pool_.start_threads();
}

void tcp_client_pool::add_client(int fd) {
  tcp_pool_.add_client(fd);
}

void tcp_client_pool::push_event(const Event & event) {

  for (auto & queue: thread_event_queues_) {
    queue->push(event);
    // TODO signal threads
  }

}

void tcp_client_pool::create_conn(int fd, async_loop & loop,
                                 tcp_connection & conn)
{
  fd_conn_data_t conn_data(conn, fd_event_queue_t{});

  fd_event_queues_[loop.id()].insert({fd, std::move(conn_data)});
}

void tcp_client_pool::data_ready(async_fd & async, tcp_connection & tcp_conn) {

  auto & fd_conn = fd_event_queues_[async.loop().id()];
  auto it = fd_conn.find(async.fd());

  CHECK(it != fd_conn.end()) << "couldn't find fd";

  auto & out_queue = it->second.second;

  if (tcp_conn.bytes_to_write == 0 && out_queue.empty()) {
    VLOG(3) << "fd: " << async.fd() << " nothing to send.";
    async.loop().set_fd_mode(async.fd(), async_fd::read);
    return;
  }

  if (!async.ready_write()) {
    VLOG(3) << "fd: " << async.fd() << " is not ready to write.";
    return;
  }

  if (!tcp_conn.write()) {
    return;
  }

  if (tcp_conn.close_connection) {

    VLOG(1) << "Closing tcp_client connection";
    fd_conn.erase(it);

    return;
  }

  while (!out_queue.empty()) {

    auto & event = out_queue.front();

    auto out_ptr = reinterpret_cast<char *>(&event[0]);
    auto out_len = event.size();

    if (!tcp_conn.copy_to_write_buffer(out_ptr, out_len)) {
      break;
    }

    async.loop().set_fd_mode(async.fd(), async_fd::readwrite);
    out_queue.pop();

  }

}

void tcp_client_pool::async(async_loop & loop) {

  size_t loop_id =  loop.id();
  VLOG(3) << "loop_id: " << loop_id;

  auto event_queue = thread_event_queues_[loop_id];

  while (!event_queue->empty()) {

    Event event;
    if (!event_queue->try_pop(event)) {
      continue;
    }

    auto output_event = output_event_fn_(event);

    for (auto & fd_conn : fd_event_queues_[loop_id]) {

      auto & tcp_conn = fd_conn.second.first;
      auto & out_queue = fd_conn.second.second;

      auto out_ptr = reinterpret_cast<char *>(&output_event[0]);
      auto out_len = output_event.size();

      if (out_queue.empty() && tcp_conn.copy_to_write_buffer(out_ptr, out_len))
      {
        loop.set_fd_mode(fd_conn.first, async_fd::readwrite);
        continue;
      }

      if (out_queue.size() > k_queue_capacity) {
        VLOG(1) << "Per connection event queue is full";
      } else {
        out_queue.push(output_event);
      }

    }
  }

}
