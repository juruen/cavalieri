#include <unordered_map>
#include <memory>
#include <glog/logging.h>
#include "streams.h"
#include "util.h"

void call_rescue(e_t e, const children_t& children) {
  for (auto& s: children) {
    s(e);
  }
}

stream_t prn() {
  return [](e_t e) {
    LOG(INFO) << "prn() " <<  event_to_json(e);
  };
}

stream_t with(const with_changes_t& changes, const children_t& children) {
  return [=](e_t e) {
    Event ne(e);
    for (auto &kv: changes) {
      VLOG(3) << "with()  first: " << kv.first << " second: " << kv.second;
      set_event_value(ne, kv.first, kv.second, true);
    }
    call_rescue(ne, children);
  };
}

stream_t with_ifempty(
    const with_changes_t& changes,
    const children_t& children)
{
  return [=](e_t e) {
    Event ne(e);
    for (auto &kv: changes) {
      VLOG(3) << "default()  first: " << kv.first << " second: " << kv.second;
      set_event_value(ne, kv.first, kv.second, false);
    }
    call_rescue(ne, children);
  };
}

stream_t split(const split_clauses_t clauses,
               const children_t& default_children)
{
  return [=](e_t e) {
    VLOG(3) << "split()";
    for (auto const &pair: clauses) {
      if (pair.first(e)) {
        VLOG(3) << "split() clause found";
        call_rescue(e, pair.second);
        return;
      }
    }
    if (default_children.size() > 0) {
      VLOG(3) << "split() default";
      call_rescue(e, default_children);
    }
  };
}

typedef std::unordered_map<std::string, const children_t> by_streams_map_t;

stream_t by(const by_keys_t& keys, const by_streams_t& streams) {
  by_streams_map_t streams_map;

  return [=](e_t e) mutable {
    VLOG(3) << "by()";

    if (keys.size() == 0) {
      return;
    }

    std::string key("");
    for (auto &k: keys) {
      std::string value = string_to_value(e, k);
      if (value == "__nil__") {
        LOG(ERROR) << "by() field empty: " << k;
      }
      key += value + "-";
    }

    VLOG(3) << "by() key: " << key;
    auto it = streams_map.find(key);
    if (it == streams_map.end()) {
      VLOG(3) << "by() key not found. Creating stream.";
      children_t children;
      for (auto &s: streams) {
        children.push_back(s());
      }
      streams_map.insert({key, std::move(children)});
      call_rescue(e, streams_map.find(key)->second);
    } else {
      VLOG(3) << "by() stream exists. ";
      call_rescue(e, it->second);
    }
  };
}

stream_t where(const predicate_t& predicate, const children_t& children,
               const children_t& else_children)
{
  return [=](e_t e) {
    if (predicate(e)) {
      call_rescue(e, children);
    } else {
      call_rescue(e, else_children);
    }
  };
}


stream_t rate(const int interval, const children_t& children) {
  double rate = 0;
  std::shared_ptr<callback_timer> timer(std::make_shared<callback_timer>(
      interval,
     [=]() mutable
      {
        VLOG(3) << "rate-timer()";
        Event e;
        e.set_metric_f(rate / interval);
        e.set_time(time(0));
        rate = 0;
        VLOG(3) << "rate-timer() value: " << e.metric_f();
        call_rescue(e, children);
      }));

  return [=](e_t e) mutable {
    VLOG(3) << "rate() rate += e.metric";
    rate += metric_to_double(e);
    (void)(timer);
  };
}

stream_t changed_state(std::string initial, const children_t& children) {
  std::string last_state(initial);

  return [=](e_t e) mutable {
    VLOG(3) << "changed_state() last_state: "
            << last_state << " new state " << e.state();
    if (last_state != e.state()) {
      VLOG(3) << "changed_state() state change";
      last_state.assign(e.state());
      call_rescue(e, children);
    }
  };
}

stream_t tagged_any(const tags_t& tags, const children_t& children) {
  return [=](e_t e) {
    VLOG(3) << "tagged_any()";
    if (tagged_any_(e, tags)) {
      VLOG(3) << "tagged_any() match";
      call_rescue(e, children);
    }
  };
}

stream_t tagged_all(const tags_t& tags, const children_t& children) {
  return [=](e_t e) {
    VLOG(3) << "tagged_all()";
    if (tagged_all_(e, tags)) {
      VLOG(3) << "tagged_all() match";
      call_rescue(e, children);
    }
  };
}

bool tagged_any_(e_t e, const tags_t& tags) {
  for (auto &tag: tags) {
    if (tag_exists(e, tag)) {
      return true;
    }
  }
  return false;
}

bool tagged_all_(e_t e, const tags_t& tags) {
  for (auto &tag: tags) {
    if (!tag_exists(e, tag)) {
      return false;
    }
  }
  return true;
}

stream_t send_index(class index& idx) {
  return [&](e_t e) {
    if (e.state() != "expired") {
      idx.add_event(e);
    }
  };
}

void streams::add_stream(stream_t stream) {
  VLOG(3) << "adding stream";
  streams_.push_back(stream);
}

void streams::process_message(const Msg& message) {
  VLOG(3) << "process message. num of streams " << streams_.size();
  unsigned long int sec = time(0);
  for (int i = 0; i < message.events_size(); i++) {
    const Event &event = message.events(i);
    if (!event.has_time()) {
      Event nevent(event);
      nevent.set_time(sec);
      push_event(std::move(nevent));
    } else {
      push_event(event);
    }
  }
}

inline void streams::push_event(const Event& e) {
  for (auto& s: streams_) {
    s(e);
  }
}


