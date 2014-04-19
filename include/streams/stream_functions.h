#ifndef STREAM_FUNCTIONS_H
#define STREAM_FUNCTIONS_H

#include <vector>
#include <list>
#include <functional>
#include <memory>
#include <boost/variant.hpp>
#include <proto.pb.h>
#include <streams/stream_infra.h>
#include <index/index.h>

typedef const Event& e_t;
typedef std::vector<Event> events_t;

#define PRED(EXP) [=](e_t e) { return (EXP); }
#define TR(EXP) [](Event & e) {(EXP); }
#define BY(EXP) []() { return (EXP); }

typedef std::function<bool(e_t)> predicate_t;
typedef std::function<void(Event &)> smap_fn_t;
typedef std::function<Event(const events_t)> fold_fn_t;
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

streams_t service(const std::string service);

streams_t service_any(const std::vector<std::string> services);

streams_t service_like(const std::string pattern);

streams_t service_like_any(const std::vector<std::string> patterns);

streams_t set_state(const std::string state);

streams_t set_metric(const double metric);

streams_t with(const with_changes_t& changes);

streams_t default_to(const with_changes_t& changes);

streams_t split(const split_clauses_t clauses);

streams_t split(const split_clauses_t clauses, const streams_t default_stream);

streams_t where(const predicate_t& predicate,
                    const streams_t else_stream);

streams_t where(const predicate_t& predicate);

streams_t by(const by_keys_t& keys, const by_stream_t stream);

streams_t rate(const int seconds);

streams_t coalesce(fold_fn_t);

streams_t project(const predicates_t predicates, fold_fn_t);

streams_t changed_state(std::string initial);

streams_t tagged_any(const tags_t& tags);

streams_t tagged_all(const tags_t& tags);

streams_t smap(smap_fn_t f);

streams_t moving_event_window(size_t window, fold_fn_t);

streams_t fixed_event_window(size_t window, fold_fn_t);

streams_t moving_time_window(time_t dt, fold_fn_t);

streams_t fixed_time_window(time_t dt, fold_fn_t);

streams_t stable(time_t dt);

streams_t throttle(size_t n, time_t dt);

streams_t above(double m);

streams_t under(double m);

streams_t within(double a, double b);

streams_t without(double a, double b);

streams_t scale(double s);

streams_t sdo();

streams_t counter();

streams_t expired();

streams_t tag(tags_t tags);

streams_t send_index();

streams_t send_graphite(const std::string host, const int port);

streams_t forward(const std::string host, const int port);

predicate_t above_eq_pred(const double value);

predicate_t above_pred(const double value);

predicate_t under_eq_pred(const double value);

predicate_t under_pred(const double value);

predicate_t state_pred(const std::string state);

predicate_t service_pred(const std::string state);

predicate_t match_pred(const std::string key, const std::string value);

predicate_t match_any_pred(const std::string key,
                          const std::vector<std::string> values);

predicate_t match_re_pred(const std::string key, const std::string value);

predicate_t match_re_any_pred(const std::string key,
                              const std::vector<std::string> values);

predicate_t match_like_pred(const std::string key, const std::string value);

predicate_t match_like_any_pred(const std::string key,
                              const std::vector<std::string> values);

predicate_t default_pred();

bool tagged_any_(e_t e, const tags_t& tags);

bool tagged_all_(e_t e, const tags_t& tags);

bool expired_(e_t e);

bool above_eq_(e_t e, const double value);

bool above_(e_t e, const double value);

bool under_eq_(e_t e, const double value);

bool under_(e_t e, const double value);

bool match_(e_t e, const std::string key, const std::string value);

bool match_re_(e_t e, const std::string key, const std::string value);

bool match_like_(e_t e, const std::string key, const std::string value);


class streams {
public:
  void add_stream(streams_t stream);
  void process_message(const Msg& message);
  void push_event(const Event& e);

private:
  std::vector<streams_t> streams_;
};



#endif
