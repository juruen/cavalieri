#ifndef CAVALIERI_STREAM_FUNCTIONS_H
#define CAVALIERI_STREAM_FUNCTIONS_H

#include <vector>
#include <list>
#include <functional>
#include <memory>
#include <boost/variant.hpp>
#include <common/event.h>
#include <streams/stream_infra.h>
#include <index/index.h>
#include <instrumentation/instrumentation.h>

typedef const Event& e_t;
typedef std::vector<Event> events_t;

#define WITH(EXP) \
  create_stream([=](const Event & oe) -> events_t { auto e = oe.copy(); (EXP); return {e}; })

#define sdo(...) svec({__VA_ARGS__})


typedef std::function<bool(e_t)> predicate_t;
typedef std::function<void(Event &)> smap_fn_t;
typedef std::function<Event(const events_t &)> fold_fn_t;
typedef std::vector<predicate_t> predicates_t;
typedef std::pair<const predicate_t, const streams_t> split_pair_t;
typedef boost::variant<std::string, int, double> change_value_t;
typedef std::map<std::string, change_value_t> with_changes_t;
typedef std::vector<std::string> by_keys_t;
typedef std::function<streams_t()> by_stream_t;
typedef std::vector<split_pair_t> split_clauses_t;
typedef std::vector<std::string> tags_t;

streams_t prn();

streams_t prn(const std::string prefix);

streams_t null();

streams_t service(const std::string service);

streams_t service_any(const std::vector<std::string> services);

streams_t service_like(const std::string pattern);

streams_t service_like_any(const std::vector<std::string> patterns);

streams_t has_attribute(const std::string attribute);

streams_t state(const std::string state);

streams_t state_any(const std::vector<std::string> states);

streams_t set_service(const std::string service);

streams_t set_state(const std::string state);

streams_t set_host(const std::string host);

streams_t set_metric(const double metric);

streams_t set_description(const std::string description);

streams_t set_ttl(const float ttl);

streams_t default_service(const std::string service);

streams_t default_state(const std::string state);

streams_t default_host(const std::string host);

streams_t default_metric(const double metric);

streams_t default_description(const std::string description);

streams_t default_ttl(const float ttl);

streams_t with(const with_changes_t& changes);

streams_t default_to(const with_changes_t& changes);

streams_t split(const split_clauses_t clauses);

streams_t split(const split_clauses_t clauses, const streams_t default_stream);

streams_t where(const predicate_t& predicate, const streams_t else_stream);

streams_t where(const predicate_t& predicate);

streams_t by(const by_keys_t& keys, const streams_t stream);

streams_t by(const by_keys_t& keys);

streams_t rate(const int seconds);

streams_t coalesce(fold_fn_t);

streams_t project(const predicates_t predicates, fold_fn_t);

streams_t changed_state(std::string initial);

streams_t changed_state();

streams_t tagged_any(const tags_t& tags);

streams_t tagged_all(const tags_t& tags);

streams_t tagged(const std::string tag);

streams_t smap(smap_fn_t f);

streams_t moving_event_window(size_t window, fold_fn_t);

streams_t fixed_event_window(size_t window, fold_fn_t);

streams_t moving_time_window(time_t dt, fold_fn_t);

streams_t fixed_time_window(time_t dt, fold_fn_t);

streams_t stable(time_t dt);

streams_t throttle(size_t n, time_t dt);

streams_t percentiles(time_t interval, std::vector<double> percentiles);

streams_t above(double m);

streams_t under(double m);

streams_t within(double a, double b);

streams_t without(double a, double b);

streams_t scale(double s);

streams_t svec(std::vector<streams_t> streams);

streams_t counter();

streams_t expired();

streams_t not_expired();

streams_t tag(tags_t tags);

streams_t ddt();

streams_t send_index();

streams_t send_graphite(const std::string host, const int port);

streams_t forward(const std::string host, const int port);

streams_t email(const std::string server, const std::string from,
                const std::string to);

streams_t email(const std::string server, const std::string from,
                const std::vector<std::string> to);

streams_t pagerduty_resolve(const std::string key);

streams_t pagerduty_acknowledge(const std::string key);

streams_t pagerduty_trigger(const std::string key);


class streams {
public:
  streams(instrumentation &);
  void add_stream(streams_t stream);
  void process_message(const riemann::Msg& message);
  void push_event(const Event& e);
  void stop();

private:
  instrumentation & instrumentation_;
  int rate_id_;
  int latency_id_;
  int in_latency_id_;
  std::vector<streams_t> streams_;
  bool stop_;
};



#endif
