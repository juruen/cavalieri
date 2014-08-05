#include <streams/stream_functions.h>
#include <external/email.h>
#include <external/pagerduty.h>
#include <rules/util.h>

target_t create_targets(const std::string pagerduty_key, const std::string to) {
  target_t target;

  auto pg_stream = (state("ok")       >> pd_resolve(pagerduty_key),
                    state("critical") >> pd_trigger(pagerduty_key));

  auto mail_stream = email("localhost", "cavalieri@localhost", to);

  target.pagerduty = changed_state("ok") >> pg_stream;
  target.email = changed_state("ok") >> mail_stream;
  target.index = send_index();
  target.all = (target.pagerduty, target.email, target.index);

  return target;
}
