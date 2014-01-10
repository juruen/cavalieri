#ifndef STREAMS_H
#define STREAMS_H

#include <vector>
#include <vector>
#include <functional>
#include <memory>
#include <boost/variant.hpp>
#include <proto.pb.h>
#include <index.h>

typedef const Event& e_t;

#define PRED(EXP) [](e_t e) { return (EXP); }
#define TR(EXP) [](Event & e) {(EXP); }
#define CHILD(EXP) {EXP}
#define BY(EXP) []() { return (EXP); }

typedef std::function<void(e_t)> stream_t;
typedef std::function<bool(e_t)> predicate_t;
typedef std::function<void(Event &)> smap_fn_t;
typedef std::vector<stream_t> children_t;
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

stream_t prn();

stream_t with(const with_changes_t& changes, const children_t& children);

stream_t with_ifempty(const with_changes_t& changes, const children_t& children);

stream_t where(const predicate_t& predicate, const children_t& children,
               const children_t& else_children={});

stream_t split(const split_clauses_t clauses,
               const stream_t& default_stream={});

stream_t by(const by_keys_t& keys, const by_streams_t& streams);

stream_t rate(const int seconds, const children_t& children);

stream_t changed_state(std::string initial, const children_t& children);

stream_t tagged_any(const tags_t& tags, const children_t& children);

stream_t tagged_all(const tags_t& tags, const children_t& children);

stream_t smap(smap_fn_t f, const children_t& children);

stream_t moving_event_window(size_t window, const children_t& children);

stream_t fixed_event_window(size_t window, const children_t& children);

stream_t moving_time_window(time_t dt, const children_t& children);

stream_t fixed_time_window(time_t dt, const children_t& children);

stream_t tag(tags_t tags, const children_t& children);

stream_t send_index(class index&);

bool tagged_any_(e_t e, const tags_t& tags);

bool tagged_all_(e_t e, const tags_t& tags);


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
