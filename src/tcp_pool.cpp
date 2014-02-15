#include <glog/logging.h>
#include <functional>
#include <tcp_pool.h>
#include <tcpconnection.h>
#include <util.h>

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

tcp_pool::tcp_pool(
    size_t thread_num,
    hook_fn_t run_fn,
    tcp_create_conn_fn_t tcp_create_conn_fn,
    tcp_ready_fn_t tcp_ready_fn)
:
  thread_pool_(thread_num),
  mutexes_(thread_num),
  new_fds_(thread_num),
  conn_maps_(thread_num),
  tcp_create_conn_fn_(tcp_create_conn_fn),
  tcp_ready_fn_(tcp_ready_fn)
{
  VLOG(3) << "tcp_pool() size: " << thread_num;
  thread_pool_.set_async_hook(std::bind(&tcp_pool::async_hook, this, _1));
  thread_pool_.set_run_hook(run_fn);
}

tcp_pool::tcp_pool(
    size_t thread_num,
    hook_fn_t run_fn,
    tcp_create_conn_fn_t tcp_create_conn_fn,
    tcp_ready_fn_t tcp_ready_fn,
    const float interval,
    timer_cb_fn_t timer_cb_fn)
:
  thread_pool_(thread_num ,interval, timer_cb_fn),
  mutexes_(thread_num),
  new_fds_(thread_num),
  conn_maps_(thread_num),
  tcp_create_conn_fn_(tcp_create_conn_fn),
  tcp_ready_fn_(tcp_ready_fn)
{
  VLOG(3) << "tcp_pool() size: " << thread_num;
  thread_pool_.set_async_hook(std::bind(&tcp_pool::async_hook, this, _1));
  thread_pool_.set_run_hook(run_fn);
}

void tcp_pool::add_client(const int fd) {
  VLOG(3) << "add_client() sfd: " << fd;

  size_t tid = thread_pool_.next_thread();
  VLOG(3) << "next thread: " << tid;
  mutexes_[tid].lock();
  new_fds_[tid].push(fd);
  mutexes_[tid].unlock();

  thread_pool_.signal_thread(tid);
}

void tcp_pool::async_hook(async_loop & loop) {
  size_t tid = loop.id();
  VLOG(3) << "async_hook() tid: " << tid;

  mutexes_[tid].lock();
  std::queue<int> new_fds(std::move(new_fds_[tid]));
  mutexes_[tid].unlock();

  while (!new_fds.empty()) {
    const int fd = new_fds.front();
    new_fds.pop();
    auto socket_cb = std::bind(&tcp_pool::socket_callback, this, _1);
    loop.add_fd(fd, async_fd::read, socket_cb);
    auto insert = conn_maps_[tid].insert({fd, tcp_connection(fd)});
    tcp_create_conn_fn_(fd, loop, insert.first->second);
    VLOG(3) << "async_hook() tid: " << tid << " adding fd: " << fd;
  }
}

void tcp_pool::socket_callback(async_fd & async) {
  auto tid = async.loop().id();
  VLOG(3) << "socket_callback() tid: " << tid;

  if (async.error()) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  auto it  = conn_maps_[tid].find(async.fd());
  CHECK(it != conn_maps_[tid].end()) << "fd not found";
  auto & conn = it->second;

  tcp_ready_fn_(async, conn);

  if (conn.close_connection) {
    VLOG(3) << "socket_callback() close_connection fd: " << async.fd();
    async.stop();
    conn_maps_[tid].erase(async.fd());
    return;
  }

  async.set_mode(conn_to_mode(conn));

  VLOG(3) << "--socket_callback() tid: " << tid;
}

void tcp_pool::start_threads() {
  thread_pool_.start_threads();
}

void tcp_pool::stop_threads() {
  thread_pool_.stop_threads();
}

tcp_pool::~tcp_pool() {
  thread_pool_.stop_threads();
}
