#include <index/mock_index.h>
#include <scheduler/mock_scheduler.h>
#include <core/mock_core.h>

mock_core::mock_core()
  :
    sched_(new mock_scheduler()),
    mock_index_(new mock_index()),
    externals_(new mock_external())
{
}

void mock_core::start() { }

void mock_core::add_stream(std::shared_ptr<streams_t>) {}

class index_interface & mock_core::idx() {
  return *mock_index_;
}

mock_index & mock_core::mock_index_impl() {
  return *mock_index_;
}

scheduler_interface & mock_core::sched() {
  return *sched_;
}

external_interface & mock_core::externals() {
  return *externals_;
}

mock_external & mock_core::mock_external_impl() {
  return *externals_;
}

void start_core(int, char**) { }
