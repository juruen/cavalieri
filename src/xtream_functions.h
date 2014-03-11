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
typedef std::pair<const predicate_t, const xtream_node_t> split_pair_t;
typedef boost::variant<std::string, int, double> change_value_t;
typedef std::map<std::string, change_value_t> with_changes_t;
typedef std::vector<std::string> by_keys_t;
typedef std::function<xtream_node_t()> by_stream_t;
typedef std::vector<split_pair_t> split_clauses_t;
typedef std::vector<std::string> tags_t;

xtream_node_t prn();

xtream_node_t with(const with_changes_t& changes);

xtream_node_t default_to(const with_changes_t& changes);

xtream_node_t split(const split_clauses_t clauses,
                    const xtream_node_t default_stream={});

xtream_node_t where(const predicate_t& predicate,
                    const xtream_node_t else_xtream);

xtream_node_t where(const predicate_t& predicate);

xtream_node_t by(const by_keys_t& keys, const by_stream_t stream);

xtream_node_t rate(const int seconds);

xtream_node_t coalesce(fold_fn_t);

xtream_node_t project(const predicates_t predicates, fold_fn_t);

xtream_node_t changed_state(std::string initial);

xtream_node_t tagged_any(const tags_t& tags);

xtream_node_t tagged_all(const tags_t& tags);

xtream_node_t smap(smap_fn_t f);

xtream_node_t moving_event_window(size_t window, fold_fn_t);

xtream_node_t fixed_event_window(size_t window, fold_fn_t);

xtream_node_t moving_time_window(time_t dt, fold_fn_t);

xtream_node_t fixed_time_window(time_t dt, fold_fn_t);

xtream_node_t stable(time_t dt);

xtream_node_t throttle(size_t n, time_t dt);

xtream_node_t above(double m);

xtream_node_t under(double m);

xtream_node_t within(double a, double b);

xtream_node_t without(double a, double b);

xtream_node_t scale(double s);

xtream_node_t sdo();

xtream_node_t counter();

xtream_node_t expired();

xtream_node_t tag(tags_t tags);

xtream_node_t send_index(class index&);

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
  void add_stream(xtream_node_t stream);
  void process_message(const Msg& message);
  void push_event(const Event& e);

private:
  std::vector<xtream_node_t> streams_;
};



#endif
