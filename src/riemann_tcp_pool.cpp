#include <riemann_tcp_pool.h>
#include <riemanntcpconnection.h>

riemann_tcp_pool::riemann_tcp_pool(size_t thread_num, incoming_events & ievents)
:
  tcp_pool_(thread_num)
{
  tcp_pool_.set_tcp_conn_hook(
      [&](int fd)
        {
          return std::make_shared<riemann_tcp_connection>(fd, ievents);
        }
  );
  tcp_pool_.start_threads();
}

void riemann_tcp_pool::add_client(int fd) {
  tcp_pool_.add_client(fd);
}


