#ifndef CORE_MOCK_CORE_H
#define CORE_MOCK_CORE_H

#include <core/core.h>
#include <index/mock_index.h>

class mock_core : public core_interface {
public:
  mock_core();
  void start();
  void add_stream(std::shared_ptr<streams_t> stream);
  std::shared_ptr<class index> index();
  std::shared_ptr<mock_index> mock_index_impl();

private:
  std::shared_ptr<mock_index> mock_index_;
  std::shared_ptr<class index> index_;
};

#endif
