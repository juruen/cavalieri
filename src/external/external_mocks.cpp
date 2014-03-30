#include <external_mocks.h>
#include <streams/stream_infra.h>
#include <scheduler/scheduler.h>

void external_mocks::add_call(const std::string & external,
                              const std::string & message,
                              const std::string & extra, const Event & e) {
  calls_.push_back({external, message, extra, g_scheduler.unix_time(), e});
}

std::vector<external_event_t> external_mocks::calls() const {
  return calls_;
}

streams_t pd_trigger(const std::string & pgkey) {
  return create_stream(
    [=](forward_fn_t, const Event & e) {
       g_external_mocks.add_call("pagerduty",
                                 "trigger pagerduty for key " + pgkey, "", e);
    });
}

streams_t pd_resolve(const std::string & pgkey) {
  return create_stream(
    [=](forward_fn_t, const Event & e) {
       g_external_mocks.add_call("pagerduty",
                                 "resolve pagerduty for key " + pgkey, "", e);
    });
}

streams_t pd_acknowledge(const std::string & pgkey) {
  return create_stream(
    [=](forward_fn_t, const Event & e) {
       g_external_mocks.add_call("pagerduty",
                                 "acknowledge pagerduty for key " + pgkey,
                                 "", e);
    });
}

streams_t email(const std::string &,
                const std::string & from, const std::string & to)
{
  return create_stream(
    [=](forward_fn_t, const Event & e) {
      g_external_mocks.add_call("email",
                                "send email from: " + from + " to: " + to,
                                "", e);
  });
}
