#include <glog/logging.h>
#include <riemann_tcp_pool.h>
#include <riemann_tcp_connection.h>

using namespace std::placeholders;

namespace {

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

riemann_tcp_pool::riemann_tcp_pool(size_t thread_num, raw_msg_fn_t raw_msg_fn)
:
  tcp_pool_(thread_num,
            {},
            std::bind(&riemann_tcp_pool::create_conn, this, _1, _2, _3),
            std::bind(&riemann_tcp_pool::data_ready, this, _1, _2)),
  raw_msg_fn_(raw_msg_fn),
  connections_(thread_num)
{
  tcp_pool_.start_threads();
}

void riemann_tcp_pool::add_client(int fd) {
  tcp_pool_.add_client(fd);
}

void riemann_tcp_pool::create_conn(int fd, async_loop & loop,
                                   tcp_connection & conn)
{

  auto & fd_conn = connections_[loop.id()];

  fd_conn.insert({fd, riemann_tcp_connection(conn, raw_msg_fn_)});
}

void riemann_tcp_pool::data_ready(async_fd & async, tcp_connection & tcp_conn) {

  auto & fd_conn = connections_[async.loop().id()];

  auto it = fd_conn.find(async.fd());
  CHECK(it != fd_conn.end()) << "fd not found";

  auto riemann_conn = it->second;

  riemann_conn.callback(async);

  async.set_mode(conn_to_mode(tcp_conn));

  if (tcp_conn.close_connection) {
    fd_conn.erase(it);
  }
}

riemann_tcp_pool::~riemann_tcp_pool() {
  tcp_pool_.stop_threads();
}
