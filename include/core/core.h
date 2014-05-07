#ifndef CORE_CORE_H
#define CORE_CORE_H

#include <streams/stream_functions.h>
#include <async/async_loop.h>
#include <pub_sub/pub_sub.h>
#include <index/index.h>
#include <scheduler/scheduler.h>
#include <riemann_tcp_pool.h>
#include <riemann_udp_pool.h>
#include <websocket_pool.h>
#include <external/external.h>
#include <config/config.h>

class core_interface {
public:
  virtual void start() = 0;
  virtual void add_stream(std::shared_ptr<streams_t> stream) = 0;
  virtual std::shared_ptr<class index> index() = 0;
  virtual std::shared_ptr<scheduler> sched() = 0;
  virtual std::shared_ptr<external> externals() = 0;
  virtual ~core_interface() {};
};

class core {
public:

  core(core_interface *impl);
  void start();
  void add_stream(std::shared_ptr<streams_t> stream);
  std::shared_ptr<class index> index();
  std::shared_ptr<scheduler> sched();
  std::shared_ptr<external> externals();

  void process_event_time(const time_t t);

private:
  std::unique_ptr<core_interface> impl_;
};

extern std::shared_ptr<core> g_core;

void start_core(int argv, char **argc);

#endif
