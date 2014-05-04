#include <external/rieman_tcp_client.h>

namespace {

const size_t k_default_pool_size = 3;

}

void riemann_tcp_client::push_event(const std::string host, const int port,
                          const Event & event)
{

  pool(host, port)->push_event(event);

}

std::shared_ptr<riemann_tcp_client_pool> riemann_tcp_client::pool(
    const std::string host,
    const int port)
{

  auto it = pool_map_.find(host);
  std::shared_ptr<riemann_tcp_client_pool> pool;

  if (it == pool_map_.end()) {

    std::lock_guard<std::mutex> lock(mutex_);

    auto it1 = pool_map_.find(host);
    if (it1 == pool_map_.end()) {

      pool = std::make_shared<riemann_tcp_client_pool>(k_default_pool_size,
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
