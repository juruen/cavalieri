#ifndef STREAMS_H
#define STREAMS_H

#include <list>
#include <functional>
#include <proto.pb.h>
#include <index.h>

typedef const Event& e_t;

#define PRED(EXP) [](e_t e) { return (EXP); }
#define CHILD(EXP) {EXP}
#define BY(EXP) []() { return (EXP); }

typedef std::function<void(e_t)> stream_t;
typedef std::function<bool(e_t)> predicate_t;
typedef std::list<stream_t> children_t;
typedef std::pair<const predicate_t, const children_t> split_pair_t;
typedef std::map<std::string, std::string> with_changes_t;
typedef std::list<std::string> by_keys_t;
typedef std::function<stream_t()> by_stream_t;
typedef std::list<by_stream_t> by_streams_t;
typedef std::list<split_pair_t> split_clauses_t;
typedef std::list<std::string> tags_t;

void call_rescue(e_t e, const children_t& children);

stream_t prn();

stream_t with(const with_changes_t& changes, const children_t& children);

stream_t where(const predicate_t& predicate, const children_t& children,
               const children_t& else_children={});

stream_t split(const split_clauses_t clauses,
               const children_t& default_children={});

stream_t by(const by_keys_t& keys, const by_streams_t& streams);

stream_t rate(const int seconds, const children_t& children);

stream_t changed_state(std::string initial, const children_t& children);

stream_t tagged_any(const tags_t& tags, const children_t& children);

stream_t tagged_all(const tags_t& tags, const children_t& children);

stream_t send_index(Index& index);

bool tagged_any_(e_t e, const tags_t& tags);

bool tagged_all_(e_t e, const tags_t& tags);


struct Streams {
  std::list<stream_t> streams;
  void add_stream(stream_t stream);
  void process_message(const Msg& message);
  void push_event(const Event& e);
};

#endif
