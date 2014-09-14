#ifndef CAVALIERI_CORE_MOCK_CORE_H
#define CAVALIERI_CORE_MOCK_CORE_H

#include <core/core.h>
#include <index/mock_index.h>
#include <scheduler/mock_scheduler.h>
#include <external/mock_external.h>

class mock_core : public core_interface {
public:
  mock_core();

  void start();
  void add_stream(std::shared_ptr<streams_t> stream);
  index_interface & idx();
  scheduler_interface & sched();
  external_interface & externals();

  mock_external & mock_external_impl();
  mock_index & mock_index_impl();

private:
  std::unique_ptr<scheduler_interface> sched_;
  std::unique_ptr<mock_index> mock_index_;
  std::unique_ptr<mock_external> externals_;
};

#endif
