#include <core/core.h>
#include <external/mock_external.h>

namespace {

void add_call(std::vector<external_event_t> & calls,
              const std::string  external,
              const std::string  message,
              const std::string  extra,
              const Event e)
{
  calls.push_back({external, message, extra, g_core->sched().unix_time(), e});
}

}

void mock_external::forward(const std::string server, const int port,
                            const Event e)
{
  add_call(calls_, "forward", "forward event to " + server, "", e);
}

void mock_external::graphite(const std::string server, const int port,
                             const Event e)
{
  add_call(calls_, "graphite", "send event to " + server, "", e);
}

void mock_external::pager_duty_trigger(const std::string pg_key,
                                       const Event e)
{
  add_call(calls_, "pagerduty", "trigger pagerduty for key " + pg_key, "", e);
}

void mock_external::pager_duty_resolve(const std::string pg_key,
                                       const Event e)
{
  add_call(calls_, "pagerduty", "resolve pagerduty for key " + pg_key, "", e);
}

void mock_external::pager_duty_acknowledge(const std::string pg_key,
                                           const Event e)
{
  add_call(calls_, "pagerduty", "acknowledge pagerduty for key " + pg_key,
           "", e);
}

void mock_external::email(const std::string server, const std::string from,
                          const std::string to, const Event e)
{
  add_call(calls_, "email", "send email from " + from + " to ", "", e);
}

std::vector<external_event_t> mock_external::calls() const {
  return calls_;
}
