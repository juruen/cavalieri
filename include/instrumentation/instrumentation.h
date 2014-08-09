#ifndef CAVALIERI_INSTRUMENTATION_INSTRUMENTATION_H
#define CAVALIERI_INSTRUMENTATION_INSTRUMENTATION_H

#include <string>
#include <vector>
#include <atomic>
#include <proto.pb.h>
#include <instrumentation/rate.h>
#include <instrumentation/reservoir.h>
#include <instrumentation/gauge.h>
#include <config/config.h>

class instrumentation {
public:

  typedef unsigned int id_t;

  instrumentation(const config conf);

  id_t add_rate(const std::string service, const std::string description);
  void update_rate(const id_t id,const unsigned int ticks);

  id_t add_latency(const std::string service, const std::string description,
                  std::vector<double> percentiles);
  void update_latency(const unsigned int id, const double value);

  id_t add_gauge(const std::string service, const std::string description);
  void update_gauge(const id_t id, unsigned int value);
  void incr_gauge(const id_t id, unsigned int value);
  void decr_gauge(const id_t id, unsigned int value);

  std::vector<Event> snapshot();

private:
  void set_rates(std::vector<Event> & events, Event event);
  void set_latencies(std::vector<Event> & events, Event event);
  void set_gauges(std::vector<Event> & events, Event event);

private:

  typedef struct {
    std::string service;
    std::string description;
    class rate rate;
  } rate_conf_t;

  typedef struct {
    std::string service;
    std::string description;
    std::vector<double> percentiles;
    class reservoir reservoir;
  } percentiles_conf_t;

  typedef struct {
    std::string service;
    std::string description;
    class gauge gauge;
  } gauge_conf_t;


  std::atomic<int> rates_id_;
  std::atomic<int> latencies_id_;
  std::atomic<int> gauges_id_;

  std::vector<rate_conf_t> rates_;
  std::vector<percentiles_conf_t> latencies_;
  std::vector<gauge_conf_t> gauges_;

};

#endif
