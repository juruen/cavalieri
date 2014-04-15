#include <glog/logging.h>
#include <functional>
#include <utility>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <transport/tcp_connection.h>
#include <transport/tcp_client_pool.h>

namespace {

const size_t k_queue_capacity = 10000;
const size_t k_reconnect_interval_secs = 5;

int connect_client(const std::string host, const int port) {

  auto sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (sock_fd < 0) {
    LOG(ERROR) << "failed to create socket for graphite";
    return -1;
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_INET;

  struct addrinfo* server;

  if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints,
                  &server) != 0)
  {
    LOG(ERROR) << "failed to resolve: " << host;
    return -1;
  }

  if (connect(sock_fd, server->ai_addr, server->ai_addrlen) < 0) {
    LOG(ERROR) << "failed to connect() graphite";

    freeaddrinfo(server);
    close(sock_fd);

    return -1;
  }

  fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL, 0) | O_NONBLOCK);

  VLOG(1) << "graphite socket connected";

  return sock_fd;

}

bool add_client(const int loop_id, tcp_pool & tcp_pool,
                const std::string host, const int port) {

  auto socket_fd = connect_client(host, port);
  bool ok = socket_fd != -1;

  if (ok) {

    tcp_pool.add_client(loop_id, socket_fd);

  }

  return ok;
}


}

using namespace std::placeholders;

tcp_client_pool::tcp_client_pool(size_t thread_num, const std::string host,
                                 const int port,
                                 output_event_fn_t output_event_fn)
:
  tcp_pool_(
    thread_num,
    {},
    std::bind(&tcp_client_pool::create_conn, this, _1, _2, _3),
    std::bind(&tcp_client_pool::data_ready, this, _1, _2),
    k_reconnect_interval_secs,
    std::bind(&tcp_client_pool::timer, this, _1),
    std::bind(&tcp_client_pool::async, this, _1)
  ),
  host_(host),
  port_(port),
  thread_event_queues_(0),
  fd_event_queues_(thread_num),
  output_event_fn_(output_event_fn),
  next_thread_(0)
{

  for (size_t i = 0; i < thread_num; i++) {

    auto queue = std::make_shared<event_queue_t>();
    queue->set_capacity(k_queue_capacity);

    thread_event_queues_.push_back(queue);

  }

  tcp_pool_.start_threads();

}

void tcp_client_pool::push_event(const Event & event) {

  thread_event_queues_[next_thread_]->try_push(event);

  tcp_pool_.signal_thread(next_thread_);

  next_thread_ = (next_thread_ + 1) % thread_event_queues_.size();

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

  /* First things first, check whether the connection is closed */
  if (async.ready_read()) {

    tcp_conn.read(1);

    if (tcp_conn.close_connection) {

      fd_conn.erase(it);

      return;
    }

  }

  /* Connection is open */

  auto & out_queue = it->second.second;

  if (tcp_conn.bytes_to_write == 0 && out_queue.empty()) {

    /* There isn't anything to send, we only want read notifications. */
    async.loop().set_fd_mode(async.fd(), async_fd::read);

    return;
  }

  if (!async.ready_write()) {

    /* fd is not ready to write */
    return;
  }

  if (!tcp_conn.write()) {
    return;
  }

  if (tcp_conn.bytes_to_write > 0) {

    /* There is still data in the buffer that has not been sent */
    return;
  }

  /* Fill the buffer with more data to send */

  tcp_conn.bytes_written = 0;

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

  if (fd_event_queues_[loop_id].empty()) {
    return;
  }

  auto event_queue = thread_event_queues_[loop_id];

   while (!event_queue->empty()) {

    Event event;
    if (!event_queue->try_pop(event)) {
      continue;
    }

    auto output_event = output_event_fn_(event);

    for (auto & fd_conn : fd_event_queues_[loop_id]) {

      auto & out_queue = fd_conn.second.second;

      if (out_queue.size() > k_queue_capacity) {
        VLOG(1) << "Per connection event queue is full";
      } else {
        out_queue.push(output_event);
        loop.set_fd_mode(fd_conn.first, async_fd::readwrite);
      }

    }
  }

}

void tcp_client_pool::timer(async_loop & loop) {

  size_t loop_id =  loop.id();

  if (!fd_event_queues_[loop_id].empty()) {
    return;
  }

  add_client(loop_id, tcp_pool_, host_, port_);

}
