#include <index/mock_index.h>
#include <core/mock_core.h>

mock_core::mock_core()
  :
    mock_index_(new mock_index()),
    index_(new class index(
                      std::dynamic_pointer_cast<index_interface>(mock_index_)))
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

void start_core(int, char**) { }
