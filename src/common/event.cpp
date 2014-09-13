#include <jsoncpp/json/json.h>
#include <common/event.h>

namespace {

const double k_default_metric = 0;
const std::string k_empty_value = "nil";

void clear_metrics(riemann::Event & e) {
  e.clear_metric_d();
  e.clear_metric_f();
  e.clear_metric_sint64();
}

}

Event::Event(const riemann::Event & event) : event_(event) {
}

Event::Event() {

}

Event Event::copy() const {
  return *this;
}

riemann::Event Event::riemann_event() const {
  return event_;
}

std::string Event::host() const {
  return event_.host();
}

Event & Event::set_host(const std::string host) {
  event_.set_host(host);
  return *this;
}

bool Event::has_host() const {
  return event_.has_host();
}

Event & Event::clear_host() {
  event_.clear_host();
  return *this;
}

std::string Event::service() const {
  return event_.service();
}

Event & Event::set_service(const std::string service) {
  event_.set_service(service);
  return *this;
}

bool Event::has_service() const {
  return event_.has_service();
}

Event & Event::clear_service() {
  event_.clear_service();
  return *this;
}

std::string Event::state() const {
  return event_.state();
}

Event & Event::set_state(const std::string state) {
  event_.set_state(state);
  return *this;
}

bool Event::has_state() const {
  return event_.has_state();
}

Event & Event::clear_state() {
  event_.clear_state();
  return *this;
}

std::string Event::description() const {
  return event_.description();
}

Event & Event::set_description(const std::string description) {
  event_.set_description(description);
  return *this;
}

bool Event::has_description() const {
  return event_.has_description();
}

Event & Event::clear_description() {
  event_.clear_description();
  return *this;
}

int64_t Event::time() const {
  return event_.time();
}

Event & Event::set_time(const int64_t time) {
  event_.set_time(time);
  return *this;
}

bool Event::has_time() const {
  return event_.has_time();
}

Event & Event::clear_time() {
  event_.clear_time();
  return *this;
}

float Event::ttl() const {
  return event_.ttl();
}

Event & Event::set_ttl(const float ttl) {
  event_.set_ttl(ttl);
  return *this;
}

bool Event::has_ttl() const {
  return event_.has_ttl();
}

Event & Event::clear_ttl() {
  event_.clear_ttl();
  return *this;
}

double Event::metric() const {
  if (event_.has_metric_f()) {
    return event_.metric_f();
  } else if (event_.has_metric_sint64()) {
    return event_.metric_sint64();
  } else if (event_.has_metric_d()) {
    return event_.metric_d();
  } else {
    // TODO throw exception
    return k_default_metric;
  }
}

Event & Event::set_metric(const double metric) {
  clear_metrics(event_);
  event_.set_metric_d(metric);
  return *this;
}

bool Event::has_metric() const {
  return (event_.has_metric_f()
          || event_.has_metric_d()
          || event_.has_metric_sint64());
}

Event & Event::clear_metric() {
  clear_metrics(event_);
  return *this;
}

std::string Event::metric_to_str() const {
  if (has_metric()) {
    return std::to_string(metric());
  } else {
    return k_empty_value;
  }
}

float Event::metric_f() const {
  return event_.has_metric_f();
}

Event & Event::set_metric_f(const float metric) {
  event_.set_metric_f(metric);
  return *this;
}

bool Event::has_metric_f() const {
  return event_.has_metric_f();
}

Event & Event::clear_metric_f() {
  event_.clear_metric_f();
  return *this;
}

float Event::metric_d() const {
  return event_.metric_d();
}

Event & Event::set_metric_d(const double metric) {
  event_.set_metric_d(metric);
  return *this;
}

bool Event::has_metric_d() const {
  return event_.has_metric_d();
}

Event & Event::clear_metric_d() {
  event_.clear_metric_d();
  return *this;
}

int64_t Event::metric_sint64() const {
  return event_.metric_sint64();
}

Event & Event::set_metric_sint64(const int64_t metric) {
  event_.set_metric_sint64(metric);
  return *this;
}

bool Event::has_metric_sint64() const {
  return event_.has_metric_sint64();
}

Event & Event::clear_metric_sint64() {
  event_.clear_metric_sint64();
  return *this;
}

std::string Event::value_to_str(const std::string key) const {
  if (key == "host") {
    return host();
  } else if (key == "service") {
    return service();
  } else if (key == "description") {
    return description();
  } else if (key == "state") {
    return state();
  } else {
    if (has_attr(key)) {
      return attr(key);
    }
  }
  return k_empty_value;
}

bool Event::has_field_set(const std::string key) const {
  if (key == "host") {
    return has_host();
  } else if (key == "service") {
    return has_service();
  } else if (key == "description") {
    return has_description();
  } else if (key == "state") {
    return has_state();
  } else if (key == "metric") {
    return has_metric();
  } else if (key == "ttl") {
    return has_ttl();
  } else {
    return has_attr(key);
  }
}

std::string Event::json_str() const {
  Json::Value event;

  if (has_host()) {
    event["host"] = Json::Value(host());
  }

  if (has_service()) {
    event["service"] = Json::Value(service());
  }

  if (has_description()) {
    event["description"] = Json::Value(description());
  }

  if (has_state()) {
    event["state"] = Json::Value(state());
  }

  if (has_time()) {
    event["time"] = Json::Value(Json::UInt64(time()));
  }


  if (has_metric()) {
    event["metric"] = Json::Value(metric());
  }

  for (int i = 0; i < event_.attributes_size(); i++) {

    if (!(event_.attributes(i).has_key() && event_.attributes(i).has_value())) {
      continue;
    }

    event[event_.attributes(i).key()] = Json::Value(event_.attributes(i).value());

  }

  if (event_.tags_size() > 0) {

    auto tags = Json::Value(Json::arrayValue);

    for (int i = 0; i < event_.tags_size(); i++) {
      tags.append(Json::Value(event_.tags(i)));
    }

    event["tags"] = tags;
  }

  return event.toStyledString();
}

bool Event::has_tag(const std::string tag) const {
  for (int i = 0; i < event_.tags_size(); i++) {
    if (event_.tags(i) == tag) {
      return true;
    }
  }
  return false;
}

Event & Event::add_tag(const std::string tag) {
  *(event_.add_tags()) = tag;
  return *this;
}

Event & Event::clear_tags() {
  event_.clear_tags();
  return *this;
}

bool Event::has_attr(const std::string attribute) const {
  for (int i = 0; i < event_.attributes_size(); i++) {
    if (event_.attributes(i).key() == attribute) {
      return true;
    }
  }
  return false;
}

std::string Event::attr(const std::string attribute) const {
  if (has_attr(attribute)) {
    for (int i = 0; i < event_.attributes_size(); i++) {
      if (event_.attributes(i).key() == attribute) {
        return event_.attributes(i).value();
      }
    }
  }
  return k_empty_value;
}

Event & Event::set_attr(const std::string key, const std::string value) {
 if (has_attr(key)) {
    for (int i = 0; i < event_.attributes_size(); i++) {
      if (event_.attributes(i).key() == key) {
        event_.mutable_attributes(i)->set_value(value);
      }
    }
  } else {
    auto attr = event_.add_attributes();
    attr->set_key(key);
    attr->set_value(value);
  }
 return *this;
}

Event & Event::clear_attrs() {
  event_.clear_attributes();
  return *this;
}
