#ifndef RIEMANN_CLIENT_RIEMANN_TCP_CLIENT_H
#define RIEMANN_CLIENT_RIEMANN_TCP_CLIENT_H

#include <proto.pb.h>
#include <mutex>
#include <riemann_client/rieman_tcp_client_pool.h>

class riemann_tcp_client {
public:
  void push_event(const std::string host, const int port, const Event & event);

private:
  std::shared_ptr<riemann_tcp_client_pool> pool(const std::string host,
                                                const int port);
private:
  std::unordered_map<std::string,
                     std::shared_ptr<riemann_tcp_client_pool>> pool_map_;
  std::mutex mutex_;

};

#endif
