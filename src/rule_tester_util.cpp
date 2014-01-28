#include <glog/logging.h>
#include <jsoncpp/json/json.h>
#include "rule_tester_util.h"
#include <util.h>


std::vector<Event> json_to_events(const std::string json, bool & ok) {
  Json::Value root;
  Json::Reader reader;

  bool parsingSuccessful = reader.parse(json, root);
  if ( !parsingSuccessful ){
    LOG(FATAL) << "Failed to parse json:" << reader.getFormattedErrorMessages();
    ok = false;
    return {};
  }

  std::vector<Event> events;
  for (const auto e: root) {
    Event event;
    for (const auto & m: e.getMemberNames()) {
      VLOG(1) << m << "\n";
      if (m != "tags") {
       if (e[m].isString()) {
           set_event_value(event, m, e[m].asString(), true);
        } else if (e[m].isDouble()) {
           set_event_value(event, m, e[m].asDouble(), true);
        } else if (e[m].isIntegral()) {
           set_event_value(event, m, e[m].asInt(), true);
        }
      } else {
        for (const auto & tag: e[m]) {
          *event.add_tags() = tag.asString();
        }
      }
    }
    events.push_back(event);
  }
  ok = true;

  return events;;
}

