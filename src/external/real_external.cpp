#include <glog/logging.h>
#include <external/real_external.h>

namespace {

const std::string k_forward_service = "forward";
const std::string k_forward_description = "forward rate";

const std::string k_graphite_service = "graphite";
const std::string k_graphite_description = "graphite rate";

const std::string k_pagerduty_service = "pagerduty";
const std::string k_pagerduty_description = "pagerduty rate";

const std::string k_email_service = "email";
const std::string k_email_description = "email rate";

const size_t k_forward_id = 0;
const size_t k_graphite_id = 1;
const size_t k_pagerduty_id = 2;
const size_t k_email_id = 3;

}

real_external::real_external(const config conf, instrumentation & instr)
: external_interface(),
  instrumentation_(instr),
  riemann_tcp_client_(conf),
  graphite_(conf),
  pagerduty_(conf.pagerduty_pool_size, conf.enable_pagerduty_debug),
  email_(conf.mail_pool_size, conf.enable_mail_debug)
{
  rates_.push_back(instrumentation_.add_rate(k_forward_service,
                                             k_forward_description));

  rates_.push_back(instrumentation_.add_rate(k_graphite_service,
                                             k_graphite_description));

  rates_.push_back(instrumentation_.add_rate(k_pagerduty_service,
                                             k_pagerduty_description));

  rates_.push_back(instrumentation_.add_rate(k_email_service,
                                             k_email_description));
}

void real_external::forward(const std::string server, const int port,
                            const Event event)
{
  riemann_tcp_client_.push_event(server, port, event);
  instrumentation_.update_rate(rates_[k_forward_id], 1);
}

void real_external::graphite(const std::string server, const int port,
                             const Event event)
{
  graphite_.push_event(server, port, event);
  instrumentation_.update_rate(rates_[k_graphite_id], 1);
}

void real_external::pager_duty_trigger(const std::string pg_key,
                                       const Event event)
{
  pagerduty_.push_event(pagerduty_pool::pd_action::trigger, pg_key, event);
  instrumentation_.update_rate(rates_[k_pagerduty_id], 1);
}

void real_external::pager_duty_resolve(const std::string pg_key,
                                       const Event event)
{
  pagerduty_.push_event(pagerduty_pool::pd_action::resolve, pg_key, event);
  instrumentation_.update_rate(rates_[k_pagerduty_id], 1);
}

void real_external::pager_duty_acknowledge(const std::string pg_key,
                                           const Event event)
{
  pagerduty_.push_event(pagerduty_pool::pd_action::acknowledge, pg_key, event);
  instrumentation_.update_rate(rates_[k_pagerduty_id], 1);
}

void real_external::email(const std::string server, const std::string from,
                          const std::string to, const Event event)
{
  email_.push_event(server, from, {to}, event);
  instrumentation_.update_rate(rates_[k_email_id], 1);
}

void real_external::stop() {

  VLOG(3) << "stop()";

  pagerduty_.stop();
  email_.stop();

}
