#include <glog/logging.h>
#include <util.h>
#include <streams/stream_infra.h>
#include <core/core.h>

streams_t email(const std::string & server, const std::string & from,
                const std::string & to)
{

  return create_stream(

    [=](forward_fn_t, const Event & event)
    {

      g_core->externals().email(server, from, {to}, event);

    });
}
