#ifndef XTREAM_FUNCTIONS_H
#define XTREAM_FUNCTIONS_H

#include <vector>
#include <list>
#include <functional>
#include <memory>
#include <boost/variant.hpp>
#include <proto.pb.h>
#include <xtreams.h>
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
typedef std::pair<const predicate_t, const xtreams_t> split_pair_t;
typedef boost::variant<std::string, int, double> change_value_t;
typedef std::map<std::string, change_value_t> with_changes_t;
typedef std::vector<std::string> by_keys_t;
typedef std::function<xtreams_t()> by_stream_t;
typedef std::vector<split_pair_t> split_clauses_t;
typedef std::vector<std::string> tags_t;

xtreams_t prn();

xtreams_t with(const with_changes_t& changes);

xtreams_t default_to(const with_changes_t& changes);

xtreams_t split(const split_clauses_t clauses);

xtreams_t split(const split_clauses_t clauses, const xtreams_t default_stream);

xtreams_t where(const predicate_t& predicate,
                    const xtreams_t else_xtream);

xtreams_t where(const predicate_t& predicate);

xtreams_t by(const by_keys_t& keys, const by_stream_t stream);

xtreams_t rate(const int seconds);

xtreams_t coalesce(fold_fn_t);

xtreams_t project(const predicates_t predicates, fold_fn_t);

xtreams_t changed_state(std::string initial);

xtreams_t tagged_any(const tags_t& tags);

xtreams_t tagged_all(const tags_t& tags);

xtreams_t smap(smap_fn_t f);

xtreams_t moving_event_window(size_t window, fold_fn_t);

xtreams_t fixed_event_window(size_t window, fold_fn_t);

xtreams_t moving_time_window(time_t dt, fold_fn_t);

xtreams_t fixed_time_window(time_t dt, fold_fn_t);

xtreams_t stable(time_t dt);

xtreams_t throttle(size_t n, time_t dt);

xtreams_t above(double m);

xtreams_t under(double m);

xtreams_t within(double a, double b);

xtreams_t without(double a, double b);

xtreams_t scale(double s);

xtreams_t sdo();

xtreams_t counter();

xtreams_t expired();

xtreams_t tag(tags_t tags);

xtreams_t send_index(class index&);

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
  void add_stream(xtreams_t stream);
  void process_message(const Msg& message);
  void push_event(const Event& e);

private:
  std::vector<xtreams_t> streams_;
};



#endif
