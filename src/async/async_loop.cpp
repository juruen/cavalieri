#include <async/async_loop.h>

async_events::async_events(std::shared_ptr<async_events_interface> impl)
:
  async_events_interface(),
  impl_(impl)
{
}

void async_events::start_loop(size_t loop_id) {
  impl_->start_loop(loop_id);
}

void async_events::signal_loop(size_t loop_id) {
  impl_->signal_loop(loop_id);
}

void async_events::stop_all_loops() {
  impl_->stop_all_loops();
}

async_loop & async_events::loop(size_t loop_id) {
  return impl_->loop(loop_id);
}

main_async_loop::main_async_loop(
  std::shared_ptr<main_async_loop_interface> impl
)
 : impl_(impl)
{
}

void main_async_loop::start() {
  impl_->start();
}

void main_async_loop::add_tcp_listen_fd(const int fd,
                                        on_new_client_fn_t on_new_client)
{
  impl_->add_tcp_listen_fd(fd, on_new_client);
}
