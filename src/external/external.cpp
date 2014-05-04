#include <external/external.h>

external::external(external_interface *impl) : impl_(impl) {}

external::external(std::shared_ptr<external_interface> impl) : impl_(impl) {}

void external::forward(const std::string server, const int port,
                        const Event event)
{
  impl_->forward(server, port, event);
}

void external::graphite(const std::string server, const int port,
                         const Event event)
{
  impl_->graphite(server, port, event);
}

void external::pager_duty_trigger(const std::string pg_key, const Event event)
{
  impl_->pager_duty_trigger(pg_key, event);
}

void external::pager_duty_resolve(const std::string pg_key, const Event event)
{
  impl_->pager_duty_resolve(pg_key, event);
}

void external::pager_duty_acknowledge(const std::string pg_key,
                                       const Event event)
{
  impl_->pager_duty_acknowledge(pg_key, event);
}

void external::email(const std::string server, const std::string from,
                      const std::string to, const Event event)
{
  impl_->email(server, from, to, event);
}
