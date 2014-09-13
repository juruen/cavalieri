#include <algorithm>
#include <glog/logging.h>
#include <jsoncpp/json/json.h>
#include "rule_tester_util.h"
#include <util/util.h>

namespace {
  bool compare_time_events(const Event & left, const Event & right) {
    return (left.time() < right.time());
  }

  std::vector<Event> & stable_sort(std::vector<Event> & events) {
    std::stable_sort(begin(events), end(events), compare_time_events);
    return events;
  }

  Json::Value event_to_jsoncpp(const Event & e) {
    Json::Value event;
    event["host"] = Json::Value(e.host());
    event["service"] = Json::Value(e.service());
    event["description"] = Json::Value(e.description());
    event["state"] = Json::Value(e.state());
    event["metric"] = Json::Value(e.metric());
    event["time"] = Json::Value::UInt64(e.time());
    event["state"] = Json::Value(e.state());

    Json::Value tags(Json::arrayValue);
    auto re = e.riemann_event();
    for (auto i = 0; i < re.tags_size(); i++ ) {
      tags.append(re.tags(i));
    }

    event["tags"] = tags;

    return event;
  }

  Json::Value index_event_to_json(mock_index_events_t idx_event) {
    Json::Value event;
    event["time"] = Json::Value::UInt64(static_cast<size_t>(idx_event.first));
    event["event"] = event_to_jsoncpp(idx_event.second);
    return event;
  }

  Json::Value index_to_json(std::vector<mock_index_events_t> idx_events) {
    Json::Value events(Json::arrayValue);
    for (const auto & idx_event: idx_events) {
      events.append(index_event_to_json(idx_event));
    }
    return events;
  }

  Json::Value external_events_to_json(std::vector<external_event_t> ex_event) {
    Json::Value events(Json::arrayValue);

    for (const auto & e: ex_event) {
      Json::Value event(Json::arrayValue);
      event.append(e.external_method);

      Json::Value ex_data(Json::objectValue);
      ex_data["time"] = Json::Value::UInt64(static_cast<size_t>(e.time));
      ex_data["message"]= e.message;
      ex_data["extra"] = e.extra;
      ex_data["event"] = event_to_jsoncpp(e.e);
      event.append(ex_data);

      events.append(event);
    }

    return events;
  }
}


std::vector<Event> json_to_events(const std::string json, bool & ok) {
  Json::Value root;
  Json::Reader reader;

  bool parsingSuccessful = reader.parse(json, root);
  if (!parsingSuccessful) {
    LOG(FATAL) << "Failed to parse json:" << reader.getFormattedErrorMessages();
    ok = false;
    return {};
  }

  std::vector<Event> events;
  for (const auto e: root) {
    Event event;
    for (const auto & m: e.getMemberNames()) {
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
          event.add_tag(tag.asString());
        }
      }
    }
    events.push_back(event);
  }
  ok = true;

  return stable_sort(events);
}

std::string results(std::vector<mock_index_events_t> index_events,
                    std::vector<external_event_t> external_events)
{
  Json::Value result;
  Json::Value reports(Json::arrayValue);
  Json::Value index(Json::arrayValue);

  result["reports"] = external_events_to_json(external_events);
  result["index"] = index_to_json(index_events);

  return result.toStyledString();
}
