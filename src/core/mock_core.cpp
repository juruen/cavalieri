#include <index/mock_index.h>
#include <scheduler/mock_scheduler.h>
#include <external/mock_external.h>
#include <core/mock_core.h>

namespace {
scheduler create_scheduler() {

  auto p = std::make_shared<mock_scheduler>();

  return scheduler(std::dynamic_pointer_cast<scheduler_interface>(p));
}

}
mock_core::mock_core()
  :
    sched_(new scheduler(create_scheduler())),
    mock_index_(new mock_index()),
    index_(new class index(
                      std::dynamic_pointer_cast<index_interface>(mock_index_))),
    mock_external_(new class mock_external()),
    externals_(new external(mock_external_))
{
}

void mock_core::start() { }

void mock_core::add_stream(std::shared_ptr<streams_t>) {}

std::shared_ptr<class index> mock_core::index() {
  return index_;
}

std::shared_ptr<mock_index> mock_core::mock_index_impl() {
  return mock_index_;
}

std::shared_ptr<class scheduler> mock_core::sched() {
  return sched_;
}

std::shared_ptr<class external> mock_core::externals() {
  return externals_;
}

std::shared_ptr<class mock_external> mock_core::mock_external() {
  return mock_external_;
}

void start_core(int, char**) { }
