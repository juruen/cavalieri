#include <async_loop.h>

main_async_loop::main_async_loop(main_async_loop_interface & impl) :
  impl_(impl)
{
}

void main_async_loop::start() {
  impl_.start();
}

void main_async_loop::add_tcp_listen_fd(const int fd,
                                        on_new_client_fn_t on_new_client)
{
  impl_.add_tcp_listen_fd(fd, on_new_client);
}
