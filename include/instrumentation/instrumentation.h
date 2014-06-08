#ifndef INSTRUMENTATION_INSTRUMENTATION_H
#define INSTRUMENTATION_INSTRUMENTATION_H

#include <string>
#include <vector>
#include <atomic>
#include <proto.pb.h>
#include <instrumentation/rate.h>
#include <instrumentation/reservoir.h>
#include <config/config.h>

class instrumentation {
public:

  instrumentation(const config conf);

  int add_rate(const std::string service, const std::string description);
  void update_rate(const unsigned int id,const unsigned int ticks);

  int add_latency(const std::string service, const std::string description,
                  std::vector<double> percentiles);

  void update_latency(const unsigned int id, const double value);

  std::vector<Event> snapshot();

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

  std::atomic<int> rates_id_;
  std::atomic<int> latencies_id_;

  std::vector<rate_conf_t> rates_;
  std::vector<percentiles_conf_t> latencies_;

};

#endif
