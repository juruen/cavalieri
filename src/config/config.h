#ifndef CONFIG_CONFIG_H
#define CONFIG_CONFIG_H

#include <string>

struct config {
  uint32_t events_port;
  size_t riemann_tcp_pool_size;
  uint32_t ws_port;
  size_t ws_pool_size;
  uint64_t index_expire_interval;
  std::string rules_directory;
};

config create_config();

#endif
