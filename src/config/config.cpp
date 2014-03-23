#include <gflags/gflags.h>
#include <glog/logging.h>
#include <config/config.h>

//FIXME conf validation

DEFINE_int32(events_port, 5555, "listening port for incoming riemann events");

DEFINE_int32(riemann_tcp_pool_size, 1, "number of threads for tcp pool");

DEFINE_int32(ws_port, 5556, "websocket listening port to query index");

DEFINE_int32(ws_pool_size, 1, "number of threads for websocket pool");

DEFINE_int32(index_expire_interval, 60,
             "interval in seconds to expire events from index");

DEFINE_string(rules_directory, ".", "directory to load rules");

config create_config() {

  struct config conf;

  conf.events_port = FLAGS_events_port;
  conf.riemann_tcp_pool_size = FLAGS_riemann_tcp_pool_size;
  conf.ws_port = FLAGS_ws_port;
  conf.ws_pool_size = FLAGS_ws_pool_size;
  conf.index_expire_interval = FLAGS_index_expire_interval;
  conf.rules_directory = FLAGS_rules_directory;

  VLOG(1) << "config:";
  VLOG(1) << "\tevents_port: " << conf.events_port;
  VLOG(1) << "\trimeann_tcp_pool_size:: " << conf.riemann_tcp_pool_size;
  VLOG(1) << "\tws_port: " << conf.ws_port;
  VLOG(1) << "\tws_pool_size: " << conf.ws_pool_size;
  VLOG(1) << "\tindex_expire_interval: " << conf.index_expire_interval;
  VLOG(1) << "\trules_directory: " << conf.rules_directory;
  VLOG(1) << "--";

  return conf;
}
