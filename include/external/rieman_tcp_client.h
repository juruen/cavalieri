#ifndef CAVALIERI_EXTERNAL_RIEMANN_TCP_CLIENT_H
#define CAVALIERI_EXTERNAL_RIEMANN_TCP_CLIENT_H

#include <common/event.h>
#include <mutex>
#include <config/config.h>
#include <external/rieman_tcp_client_pool.h>

class riemann_tcp_client {
public:
  riemann_tcp_client(const config conf);
  void push_event(const std::string host, const int port, const Event & event);

private:
  const config config_;
  std::unordered_map<std::string,
                     std::shared_ptr<riemann_tcp_client_pool>> pool_map_;
  std::mutex mutex_;

};

#endif
