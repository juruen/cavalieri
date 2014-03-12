#include <glog/logging.h>
#include <email.h>
#include <util.h>

streams_t email(const std::string & from, const std::string & to)
{
  return create_stream_node(
    [=](forward_fn_t, const Event & event) {
      VLOG(1) << "Would send email to: " << to << " from: " << from
              << " with: " << event_to_json(event);
  });
}
