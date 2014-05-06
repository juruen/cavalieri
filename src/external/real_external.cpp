#include <external/real_external.h>

real_external::real_external()
: external_interface(),
  riemann_tcp_client_(new riemann_tcp_client()),
  graphite_(new class graphite()),
  pagerduty_(new pagerduty_pool(1)),
  email_(new mailer_pool(1))
{
}

void real_external::forward(const std::string server, const int port,
                            const Event event)
{
  riemann_tcp_client_->push_event(server, port, event);
}

void real_external::graphite(const std::string server, const int port,
                             const Event event)
{
  graphite_->push_event(server, port, event);
}

void real_external::pager_duty_trigger(const std::string pg_key,
                                       const Event event)
{
  pagerduty_->push_event(pagerduty_pool::pd_action::trigger, pg_key, event);
}

void real_external::pager_duty_resolve(const std::string pg_key,
                                       const Event event)
{
  pagerduty_->push_event(pagerduty_pool::pd_action::resolve, pg_key, event);
}

void real_external::pager_duty_acknowledge(const std::string pg_key,
                                           const Event event)
{
  pagerduty_->push_event(pagerduty_pool::pd_action::acknowledge, pg_key, event);
}

void real_external::email(const std::string server, const std::string from,
                          const std::string to, const Event event)
{
  email_->push_event(server, from, {to}, event);
}
