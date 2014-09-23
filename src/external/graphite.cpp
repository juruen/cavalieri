#include <external/graphite.h>

graphite::graphite(const config conf) : config_(conf) {}

void graphite::push_event(const std::string host, const int port,
                          const Event & event)
{

  std::shared_ptr<graphite_pool> pool;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pool_map_.find(host);
    if (it == pool_map_.end()) {
      pool = std::make_shared<graphite_pool>(config_.graphite_pool_size,
                                             host, port);
      pool_map_.insert({host, pool});
    } else {
      pool = it->second;
    }
  }

  pool->push_event(event);

}
