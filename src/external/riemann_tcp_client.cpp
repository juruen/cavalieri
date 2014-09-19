#include <external/rieman_tcp_client.h>


riemann_tcp_client::riemann_tcp_client(const config conf) : config_(conf) { }

void riemann_tcp_client::push_event(const std::string host, const int port,
                          const Event & event)
{
  std::shared_ptr<riemann_tcp_client_pool> pool;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pool_map_.find(host);
    if (it == pool_map_.end()) {
      pool = std::make_shared<riemann_tcp_client_pool>(
                          config_.graphite_pool_size, host, port);
      pool_map_.insert({host, pool});
    } else {
      pool = it->second;
    }
  }

  pool->push_event(event);
}
