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
  size_t pagerduty_pool_size;
  size_t mail_pool_size;
  size_t graphite_pool_size;
  size_t forward_pool_size;
  bool enable_mail_debug;
  bool enable_pagerduty_debug;
  bool enable_internal_metrics;
};

config create_config();

void log_config(config);

#endif
