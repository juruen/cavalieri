#ifndef CAVALIERI_EXTERNAL_REAL_EXTERNAL_H
#define CAVALIERI_EXTERNAL_REAL_EXTERNAL_H

#include <config/config.h>
#include <external/external.h>
#include <external/graphite.h>
#include <external/rieman_tcp_client.h>
#include <external/pagerduty_pool.h>
#include <external/mailer_pool.h>
#include <instrumentation/instrumentation.h>

class real_external : public external_interface {
public:
  real_external(const config, instrumentation & instrumentation);
  void forward(const std::string server, const int port, const Event event);
  void graphite(const std::string server, const int port, const Event event);
  void pager_duty_trigger(const std::string pg_key, const Event event);
  void pager_duty_resolve(const std::string pg_key, const Event event);
  void pager_duty_acknowledge(const std::string pg_key, const Event event);
  void email(const std::string server, const std::string from,
             const std::string to, const Event event);
  void stop();

private:
  instrumentation & instrumentation_;
  riemann_tcp_client riemann_tcp_client_;
  class graphite graphite_;
  pagerduty_pool pagerduty_;
  mailer_pool email_;
  std::vector<size_t> rates_;
};

#endif
