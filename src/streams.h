#ifndef STREAMS_H
#define STREAMS_H

#include <vector>
#include <list>
#include <functional>
#include <memory>
#include <boost/variant.hpp>
#include <proto.pb.h>
#include <index.h>

typedef const Event& e_t;
typedef std::vector<Event> events_t;

#define PRED(EXP) [=](e_t e) { return (EXP); }
#define TR(EXP) [](Event & e) {(EXP); }
#define CHILD(EXP) {EXP}
#define S(...) std::vector<stream_t> {__VA_ARGS__}
#define BY(EXP) []() { return (EXP); }

typedef std::function<void(e_t)> stream_t;
typedef std::function<bool(e_t)> predicate_t;
typedef std::function<void(Event &)> smap_fn_t;
typedef boost::variant<stream_t, std::vector<stream_t>> children_t;
typedef std::function<void(events_t)> mstream_t;
typedef boost::variant<mstream_t, std::vector<mstream_t>> mchildren_t;
typedef std::vector<predicate_t> predicates_t;
typedef std::pair<const predicate_t, const stream_t> split_pair_t;
typedef boost::variant<std::string, int, double> change_value_t;
typedef std::map<std::string, change_value_t> with_changes_t;
typedef std::vector<std::string> by_keys_t;
typedef std::function<stream_t()> by_stream_t;
typedef std::vector<by_stream_t> by_streams_t;
typedef std::vector<split_pair_t> split_clauses_t;
typedef std::vector<std::string> tags_t;

void call_rescue(e_t e, const children_t& children);
void call_rescue(const std::vector<Event> events, const children_t& children);
void call_rescue(const std::list<Event> events, const children_t& children);

stream_t prn();

stream_t with(const with_changes_t& changes, const children_t& children);

stream_t with_ifempty(const with_changes_t& changes, const children_t& children);

stream_t where(const predicate_t& predicate, const children_t& children,
               const children_t& else_children=std::vector<stream_t>{});

stream_t split(const split_clauses_t clauses,
               const stream_t& default_stream={});

stream_t by(const by_keys_t& keys, const by_streams_t& streams);

stream_t rate(const int seconds, const children_t& children);

stream_t coalesce(const mchildren_t& children);

stream_t project(const predicates_t predicates, const mchildren_t& children);

stream_t changed_state(std::string initial, const children_t& children);

stream_t tagged_any(const tags_t& tags, const children_t& children);

stream_t tagged_all(const tags_t& tags, const children_t& children);

stream_t smap(smap_fn_t f, const children_t& children);

stream_t moving_event_window(size_t window, const mchildren_t& children);

stream_t fixed_event_window(size_t window, const mchildren_t& children);

stream_t moving_time_window(time_t dt, const mchildren_t& children);

stream_t fixed_time_window(time_t dt, const mchildren_t& children);

stream_t stable(time_t dt, const children_t& children);

stream_t throttle(size_t n, time_t dt, const children_t& children);

stream_t above(double m, const children_t& children);

stream_t under(double m, const children_t& children);

stream_t within(double a, double b, const children_t& children);

stream_t without(double a, double b, const children_t& children);

stream_t scale(double s, const children_t& children);

stream_t sdo(const children_t& children);

stream_t counter(const children_t& children);

stream_t expired(const children_t& children);

stream_t tag(tags_t tags, const children_t& children);

stream_t send_index(class index&);

predicate_t above_eq_pred(const double value);

predicate_t above_pred(const double value);

predicate_t under_eq_pred(const double value);

predicate_t under_pred(const double value);

predicate_t state_pred(std::string state);

predicate_t service_pred(std::string state);

bool tagged_any_(e_t e, const tags_t& tags);

bool tagged_all_(e_t e, const tags_t& tags);

bool expired_(e_t e);

bool above_eq_(e_t e, const double value);

bool above_(e_t e, const double value);

bool under_eq_(e_t e, const double value);

bool under_(e_t e, const double value);

class streams {
public:
  void add_stream(stream_t stream);
  void process_message(const Msg& message);
  void push_event(const Event& e);

private:
  std::vector<stream_t> streams_;
}

;

#endif
