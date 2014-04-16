#include <core/core.h>

std::shared_ptr<core> g_core;

core::core(core_interface *impl) : impl_(impl)
{
}

void core::start() {
  impl_->start();
}

void core::add_stream(std::shared_ptr<streams_t> stream) {
  impl_->add_stream(stream);
}

std::shared_ptr<class index> core::index() {
  return impl_->index();
}

std::shared_ptr<scheduler> core::sched() {
  return impl_->sched();
}

void core::send_to_graphite(const std::string host, const int port,
                            const Event & event)
{
  impl_->send_to_graphite(host, port, event);
}

void core::forward(const std::string host, const int port, const Event & event)
{
  impl_->forward(host, port, event);
}
