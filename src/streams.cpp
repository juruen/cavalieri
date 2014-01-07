#include <unordered_map>
#include <memory>
#include <atomic>
#include <glog/logging.h>
#include <streams.h>
#include <util.h>
#include <scheduler.h>
#include <atom.h>

void call_rescue(e_t e, const children_t& children) {
  for (auto& s: children) {
    s(e);
  }
}

void call_rescue(const std::vector<Event> events, const children_t& children) {
  for (auto& e: events) {
    for (auto& s: children) {
      s(e);
    }
  }
}

void call_rescue(const std::list<Event> events, const children_t& children) {
  for (auto& e: events) {
    for (auto& s: children) {
      s(e);
    }
  }
}

stream_t prn() {
  return [](e_t e) {
    LOG(INFO) << "prn() " <<  event_to_json(e);
  };
}

stream_t with(const with_changes_t & changes,
              const bool & replace,
              const children_t & children)
{
  return [=](e_t e) {
    Event ne(e);
    for (auto &kv: changes) {
      set_event_value(ne, kv.first, kv.second, replace);
    }
    call_rescue(ne, children);
  };
}

stream_t with(const with_changes_t& changes, const children_t& children) {
  return with(changes, true, children);
}

stream_t with_ifempty(
    const with_changes_t& changes,
    const children_t& children)
{
  return with(changes, false, children);
}

stream_t split(const split_clauses_t clauses,
               const stream_t& default_stream)
{
  return [=](e_t e) {
    for (auto const &pair: clauses) {
      if (pair.first(e)) {
        call_rescue(e, {pair.second});
        return;
      }
    }
    if (default_stream) {
      call_rescue(e, {default_stream});
    }
  };
}

typedef std::unordered_map<std::string, const children_t> by_streams_map_t;

stream_t by(const by_keys_t & keys, const by_streams_t & streams) {
  auto atom_streams = make_shared_atom<by_streams_map_t>();

  return [=](e_t e) mutable {
    if (keys.empty()) {
      return;
    }

    std::string key;
    for (const auto & k: keys) {
      key += string_to_value(e, k) + " ";
    }

    auto fn_value = [&]() {
      children_t children;
      for (auto & s: streams) {
        children.push_back(s());
      }
      return children;
    };

    map_on_sync_insert<std::string, const children_t>(
        atom_streams,
        key,
        fn_value,
        [&](const children_t & c) { call_rescue(e, c); }
    );
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
  auto rate = std::make_shared<std::atomic<double>>(0);
  g_scheduler.add_periodic_task(
     [=]() mutable
      {
        VLOG(3) << "rate-timer()";
        Event e;
        e.set_metric_d(rate->exchange(0) / interval);
        e.set_time(g_scheduler.unix_time());
        VLOG(3) << "rate-timer() value: " << e.metric_d();
        call_rescue(e, children);
      },
      interval
  );

  return [=](e_t e) mutable {
    double expected, newval;
    do {
      expected = rate->load();
      newval = expected + metric_to_double(e);
    } while (!rate->compare_exchange_strong(expected, newval));
  };
}

stream_t changed_state(std::string initial, const children_t& children) {
  auto prev = make_shared_atom<std::string>(initial);

  return [=](e_t e) mutable {
    prev->update(
        e.state(),
        [&](const std::string & prev, const std::string & curr)
          {
            if (prev != e.state())
              call_rescue(e, children);
          }
    );
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


stream_t smap(smap_fn_t f, const children_t& children) {
  return [=](e_t e) {
    Event ne(e);
    f(ne);
    call_rescue(ne, children);
  };
}


stream_t moving_event_window(size_t n, const children_t& children) {
  auto window = make_shared_atom<std::list<Event>>();

  return [=](e_t e) {
    window->update(
      [&](const std::list<Event> window)
        {
          auto c(window);
          c.push_back(e);
          if (c.size() == (n + 1)) {
            c.pop_front();
          }
          return std::move(c);
        },
      [&](const std::list<Event> & prev, const std::list<Event> & curr) {
        for (const auto & ev : curr) {
          call_rescue(ev, children);
        }
      }
    );
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

void streams::push_event(const Event& e) {
  for (auto& s: streams_) {
    s(e);
  }
}


