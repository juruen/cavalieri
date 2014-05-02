#include <glog/logging.h>
#include <util.h>
#include <streams/stream_infra.h>
#include <external/mailer_pool.h>

auto mail_pool = std::make_shared<mailer_pool>(1);

streams_t email(const std::string & server, const std::string & from,
                const std::string & to)
{

  return create_stream(

    [=](forward_fn_t, const Event & event)
    {
      mail_pool->push_event(server, from, {from}, event);

  });
}
