#ifndef EXTERNAL_REAL_EXTERNAL_H
#define EXTERNAL_REAL_EXTERNAL_H

#include <external/external.h>
#include <external/graphite.h>
#include <external/rieman_tcp_client.h>
#include <external/pagerduty_pool.h>
#include <external/mailer_pool.h>

class real_external : public external_interface {
public:
  real_external();
  void forward(const std::string server, const int port, const Event event);
  void graphite(const std::string server, const int port, const Event event);
  void pager_duty_trigger(const std::string pg_key, const Event event);
  void pager_duty_resolve(const std::string pg_key, const Event event);
  void pager_duty_acknowledge(const std::string pg_key, const Event event);
  void email(const std::string server, const std::string from,
             const std::string to, const Event event);
private:
  std::unique_ptr<riemann_tcp_client> riemann_tcp_client_;
  std::unique_ptr<class graphite> graphite_;
  std::unique_ptr<pagerduty_pool> pagerduty_;
  std::unique_ptr<mailer_pool> email_;
};

#endif
