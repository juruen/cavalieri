#ifndef CORE_CORE_H
#define CORE_CORE_H

#include <streams/stream_functions.h>
#include <index/index.h>
#include <scheduler/scheduler.h>
#include <external/external.h>
#include <config/config.h>

class core_interface {
public:
  virtual void start() = 0;
  virtual void add_stream(std::shared_ptr<streams_t> stream) = 0;
  virtual index_interface & idx() = 0;
  virtual scheduler_interface & sched() = 0;
  virtual external_interface & externals() = 0;
  virtual ~core_interface() {} ;
};

extern std::shared_ptr<core_interface> g_core;

void start_core(int argv, char **argc);

#endif
