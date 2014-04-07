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

