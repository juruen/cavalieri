#include <external/real_external.h>

real_external::real_external(const config conf)
: external_interface(),
  riemann_tcp_client_(conf),
  graphite_(conf),
  pagerduty_(conf.pagerduty_pool_size, conf.enable_pagerduty_debug),
  email_(conf.mail_pool_size, conf.enable_mail_debug)
{
}

void real_external::forward(const std::string server, const int port,
                            const Event event)
{
  riemann_tcp_client_.push_event(server, port, event);
}

void real_external::graphite(const std::string server, const int port,
                             const Event event)
{
  graphite_.push_event(server, port, event);
}

void real_external::pager_duty_trigger(const std::string pg_key,
                                       const Event event)
{
  pagerduty_.push_event(pagerduty_pool::pd_action::trigger, pg_key, event);
}

void real_external::pager_duty_resolve(const std::string pg_key,
                                       const Event event)
{
  pagerduty_.push_event(pagerduty_pool::pd_action::resolve, pg_key, event);
}

void real_external::pager_duty_acknowledge(const std::string pg_key,
                                           const Event event)
{
  pagerduty_.push_event(pagerduty_pool::pd_action::acknowledge, pg_key, event);
}

void real_external::email(const std::string server, const std::string from,
                          const std::string to, const Event event)
{
  email_.push_event(server, from, {to}, event);
}
