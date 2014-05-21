#include <external/graphite.h>

graphite::graphite(const config conf) : config_(conf) {}

void graphite::push_event(const std::string host, const int port,
                          const Event & event)
{

  pool(host, port)->push_event(event);

}

std::shared_ptr<graphite_pool> graphite::pool(const std::string host,
                                              const int port)
{

  auto it = pool_map_.find(host);
  std::shared_ptr<graphite_pool> pool;

  if (it == pool_map_.end()) {

    std::lock_guard<std::mutex> lock(mutex_);

    auto it1 = pool_map_.find(host);
    if (it1 == pool_map_.end()) {

      pool = std::make_shared<graphite_pool>(config_.graphite_pool_size,
                                             host, port);
      pool_map_.insert({host, pool});

    } else {

      pool = it1->second;

    }

  } else {

    pool = it->second;

  }

  return pool;
}
