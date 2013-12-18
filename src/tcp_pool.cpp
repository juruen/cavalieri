#include <glog/logging.h>
#include <functional>
#include <tcp_pool.h>
#include <tcpconnection.h>
#include <util.h>

tcp_pool::tcp_pool(size_t thread_num)
:
  thread_pool_(thread_num),
  pool_size_(thread_num),
  mutexes_(thread_num),
  new_fds_(thread_num),
  conn_maps_(thread_num)
{
  using namespace std::placeholders;
  VLOG(3) << "tcp_pool() size: " << thread_num;
  thread_pool_.set_async_hook(std::bind(&tcp_pool::async_hook,this, _1, _2));
}

void tcp_pool::add_client(const int fd) {
  VLOG(3) << "add_client() sfd: " << fd;

  size_t tid = thread_pool_.next_thread();
  mutexes_[tid].lock();
  new_fds_[tid].push(fd);
  mutexes_[tid].unlock();

  thread_pool_.signal_thread(tid);
}

void tcp_pool::async_hook(size_t tid, ev::dynamic_loop & loop) {
  VLOG(3) << "async_hook() tid: " << tid;

  mutexes_[tid].lock();
  std::queue<int> new_fds(std::move(new_fds_[tid]));
  mutexes_[tid].unlock();

  while (!new_fds.empty()) {
    const int fd = new_fds.front();
    new_fds.pop();
    auto conn = tcp_conn_hook_(fd);
    conn->io.set(loop);
    conn->io.set<tcp_pool, &tcp_pool::socket_callback>(this);
    conn->io.start(fd, ev::READ);
    conn_maps_[tid].insert({fd, conn});
    VLOG(3) << "async_hook() tid: " << tid << " adding fd: " << fd;
  }
}

void tcp_pool::socket_callback(ev::io & io, int revents) {
  auto tid = thread_pool_.ev_to_tid(io);
  VLOG(3) << "socket_callback() tid: " << tid;

  if (EV_ERROR & revents) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  auto conn = conn_maps_[tid][io.fd];
  conn->callback(revents);

  if (conn->close_connection) {
    VLOG(3) << "socket_callback() close_connection fd: " << conn->sfd;
    conn->io.stop();
    conn_maps_[tid].erase(conn->sfd);
    return;
  }

  conn->set_io();
}

void tcp_pool::set_tcp_conn_hook(tcp_conn_hook_t hook) {
  tcp_conn_hook_ = hook;
}

void tcp_pool::set_run_hook(hook_fn_t hook) {
  thread_pool_.set_run_hook(hook);
}

void tcp_pool::start_threads() {
  thread_pool_.start_threads();
}

void tcp_pool::stop_threads() {
  thread_pool_.stop_threads();
}

size_t tcp_pool::ev_to_tid(ev::timer & timer) {
  return thread_pool_.ev_to_tid(timer);
}

conn_map_t & tcp_pool::tcp_connection_map(size_t tid) {
  return conn_maps_[tid];
}

tcp_pool::~tcp_pool() {
  stop_threads();
}
