#ifndef CAVALIERI_COMMON_EVENT_H
#define CAVALIERI_COMMON_EVENT_H

#include <proto.pb.h>

class Event {
public:
  Event();

  Event(const riemann::Event & event);

  Event(const Event &) = default;

  Event copy() const;

  riemann::Event riemann_event() const;

  std::string host() const;

  Event & set_host(const std::string host);

  bool has_host() const;

  Event & clear_host();

  std::string service() const;

  Event & set_service(const std::string service);

  bool has_service() const;

  Event & clear_service();

  std::string state() const;

  Event & set_state(const std::string state);

  bool has_state() const;

  Event & clear_state();

  std::string description() const;

  Event & set_description(const std::string description);

  bool has_description() const;

  Event & clear_description();

  int64_t time() const;

  Event & set_time(const int64_t time);

  bool has_time() const;

  Event & clear_time();

  float ttl() const;

  Event & set_ttl(const float ttl);

  bool has_ttl() const;

  Event & clear_ttl();

  double metric() const;

  Event & set_metric(const double metric);

  bool has_metric() const;

  Event & clear_metric();

  std::string metric_to_str() const;

  float metric_f() const;

  Event & set_metric_f(const float metric);

  bool has_metric_f() const;

  Event & clear_metric_f();

  float metric_d() const;

  Event & set_metric_d(const double metric);

  bool has_metric_d() const;

  Event & clear_metric_d();

  int64_t metric_sint64() const;

  Event & set_metric_sint64(const int64_t metric);

  bool has_metric_sint64() const;

  Event & clear_metric_sint64();

  std::string value_to_str(const std::string field) const;

  bool has_field_set(const std::string field) const;

  std::string json_str() const;

  bool has_tag(const std::string tag) const;

  Event & add_tag(const std::string tag);

  Event & clear_tags();

  bool has_attr(const std::string attribute) const;

  std::string attr(const std::string attribute) const;

  Event & set_attr(const std::string attribute, const std::string value);

  std::vector<std::pair<std::string, std::string>> attrs() const;

  Event & clear_attrs();

private:
  riemann::Event event_;

};

#endif
