#include <gflags/gflags.h>
#include <glog/logging.h>
#include <thread>
#include <config/config.h>

//FIXME conf validation

const auto k_cores = std::thread::hardware_concurrency();

DEFINE_int32(events_port, 5555, "listening port for incoming riemann events");

DEFINE_int32(riemann_tcp_pool_size, k_cores, "number of threads for tcp pool");

DEFINE_int32(ws_port, 5556, "websocket listening port to query index");

DEFINE_int32(ws_pool_size, 4, "number of threads for websocket pool");

DEFINE_int32(index_expire_interval, 60,
             "interval in seconds to expire events from index");

DEFINE_string(rules_directory, ".", "directory to load rules");

DEFINE_int32(pagerduty_pool_size, 1, "number of threads for pagerduty pool");

DEFINE_int32(mail_pool_size, 1, "number of threads for mail pool");

DEFINE_int32(graphite_pool_size, 4, "number of threads for graphite pool");

DEFINE_int32(forward_pool_size, 4, "number of threads for forward pool");

DEFINE_bool(enable_mail_debug, false, "enable smtp libcurl debug");

DEFINE_bool(enable_pagerduty_debug, false, "enable pagerduty libcurl debug");


config create_config() {

  struct config conf;

  conf.events_port = FLAGS_events_port;
  conf.riemann_tcp_pool_size = FLAGS_riemann_tcp_pool_size;
  conf.ws_port = FLAGS_ws_port;
  conf.ws_pool_size = FLAGS_ws_pool_size;
  conf.index_expire_interval = FLAGS_index_expire_interval;
  conf.rules_directory = FLAGS_rules_directory;
  conf.pagerduty_pool_size = FLAGS_pagerduty_pool_size;
  conf.mail_pool_size = FLAGS_mail_pool_size;
  conf.graphite_pool_size = FLAGS_graphite_pool_size;
  conf.forward_pool_size = FLAGS_forward_pool_size;
  conf.enable_mail_debug = FLAGS_enable_mail_debug;
  conf.enable_pagerduty_debug = FLAGS_enable_pagerduty_debug;

  return conf;
}

void log_config(config conf) {

  VLOG(1) << "config:";
  VLOG(1) << "\tevents_port: " << conf.events_port;
  VLOG(1) << "\trimeann_tcp_pool_size:: " << conf.riemann_tcp_pool_size;
  VLOG(1) << "\tws_port: " << conf.ws_port;
  VLOG(1) << "\tws_pool_size: " << conf.ws_pool_size;
  VLOG(1) << "\tindex_expire_interval: " << conf.index_expire_interval;
  VLOG(1) << "\trules_directory: " << conf.rules_directory;
  VLOG(1) << "\tpagerduty_pool_size: " << conf.pagerduty_pool_size;
  VLOG(1) << "\tmail_pool_size: " << conf.mail_pool_size;
  VLOG(1) << "\tgraphite_pool_size: " << conf.graphite_pool_size;
  VLOG(1) << "\tforward_pool_size: " << conf.forward_pool_size;
  VLOG(1) << "\tenable_mail_debug: " << conf.enable_mail_debug;
  VLOG(1) << "\tenable_pagerduty_debug: " << conf.enable_pagerduty_debug;
  VLOG(1) << "--";

}
