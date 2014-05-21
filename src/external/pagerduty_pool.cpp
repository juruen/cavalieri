#include <glog/logging.h>
#include <jsoncpp/json/json.h>
#include <util.h>
#include <pagerduty_pool.h>

using namespace std::placeholders;

namespace {

const std::string k_pd_url = "https://events.pagerduty.com/generic/"
                             "2010-04-15/create_event.json";

struct pd_extra {
  const std::string pg_key;
  pagerduty_pool::pd_action action;
};


std::string pd_action_to_string(const pagerduty_pool::pd_action action) {

  if (action == pagerduty_pool::pd_action::trigger) {
    return "trigger";
  } else if (action == pagerduty_pool::pd_action::acknowledge) {
    return "acknowledge";
  }  else {
    return "resolve";
  }

}

};

pagerduty_pool::pagerduty_pool(const size_t thread_num, const bool enable_debug)
  :
  curl_pool_(
      thread_num,
      std::bind(&pagerduty_pool::curl_event, this, _1, _2, _3)
  ),
  enable_debug_(enable_debug)
{
}


void pagerduty_pool::push_event(const pd_action action,
                                const std::string pd_key, const Event & event)
{
  VLOG(3) << "push_event()  key: " << pd_key;

  pd_extra extra = {pd_key, action};
  curl_pool_.push_event(event, extra);

}

void pagerduty_pool::curl_event(const queued_event_t queued_event,
                                const std::shared_ptr<CURL> easy,
                                std::function<void()>&)
{

  const auto extra = boost::any_cast<pd_extra>(queued_event.extra);
  const auto & e = queued_event.event;

  Json::Value event;

  std::string desc = e.host() + " " + e.service() + " is " + e.state()
                     + "(" + metric_to_string(e) + ")";

  event ["incident_key"] = Json::Value(e.host() + "/" + e.service());
  event["service_key"] = Json::Value(extra.pg_key);
  event["event_type"] = Json::Value(pd_action_to_string(extra.action));
  event["description"] =  desc;
  event["details"] =  event_to_json(e);

  const std::string json_str(event.toStyledString());

  VLOG(3) << "sending: " << json_str;

  curl_easy_setopt(easy.get(), CURLOPT_URL, k_pd_url.c_str());
  curl_easy_setopt(easy.get(), CURLOPT_COPYPOSTFIELDS, json_str.c_str());
  curl_easy_setopt(easy.get(), CURLOPT_VERBOSE, enable_debug_);

}

void pagerduty_pool::stop() {
  VLOG(3) << "stop()";

  curl_pool_.stop();
}
