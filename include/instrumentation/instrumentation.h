#ifndef CAVALIERI_INSTRUMENTATION_INSTRUMENTATION_H
#define CAVALIERI_INSTRUMENTATION_INSTRUMENTATION_H

#include <string>
#include <vector>
#include <atomic>
#include <common/event.h>
#include <instrumentation/rate.h>
#include <instrumentation/reservoir.h>
#include <instrumentation/gauge.h>
#include <config/config.h>

namespace instrumentation {

using update_rate_fn_t = std::function<void(const unsigned int)>;
using update_latency_fn_t = std::function<void(const double)>;

typedef struct {
    const std::function<void(const unsigned int)> update_fn;
    const std::function<void(const unsigned int)> incr_fn;
    const std::function<void(const unsigned int)> decr_fn;
} update_gauge_t;

class instrumentation {
public:

  instrumentation(const config conf);

  update_rate_fn_t  add_rate(const std::string service,
                             const std::string description);

  update_latency_fn_t add_latency(const std::string service,
                                  const std::string description,
                                  std::vector<double> percentiles);

  static std::vector<Event> reservoir_to_events(reservoir & reservoir,
                                                std::vector<double> percentiles,
                                                const Event event_base);

  update_gauge_t add_gauge(const std::string service,
                           const std::string description);
  std::vector<Event> snapshot();

private:
  using id_t = unsigned int;

  void update_rate(const id_t id,const unsigned int ticks);
  void update_latency(const unsigned int id, const double value);
  void update_gauge(const id_t id, unsigned int value);
  void incr_gauge(const id_t id, unsigned int value);
  void decr_gauge(const id_t id, unsigned int value);
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

}

#endif
