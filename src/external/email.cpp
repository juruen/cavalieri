#include <glog/logging.h>
#include <email.h>
#include <util.h>

stream_t email(const std::string & from, const std::string & to)
{
  return [=](e_t event) {
    VLOG(1) << "Would send email to: " << to << " from: " << from
            << " with: " << event_to_json(event);
  };
}
