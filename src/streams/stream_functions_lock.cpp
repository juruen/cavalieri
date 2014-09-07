#include <util/util.h>
#include <queue>
#include <core/core.h>
#include <instrumentation/reservoir.h>
#include <streams/stream_functions.h>


typedef struct {
  std::unordered_map<std::string, streams_t> by_map;
  std::mutex mutex;
} by_t;


on_event_fn_t by_lock_(const by_keys_t & keys, const streams_t stream) {

  auto streams = std::make_shared<by_t>();

  return [=](e_t e) -> next_events_t
  {

     if (keys.empty()) {
       return {};
     }

     std::string key;
     for (const auto & k: keys) {
       key += event_str_value(e, k) + " ";
     }

     streams->mutex.lock();

     streams_t & s = streams->by_map[key];

     if (s.empty()) {

       s = stream;
       init_streams(s);

     }

     streams->mutex.unlock();

     return push_event(s, e);

  };

}

streams_t by_lock(const by_keys_t & keys, const streams_t stream) {
  return create_stream([=](){ return by_lock_(keys, stream); });
};

typedef struct {
  std::unordered_map<std::string, forward_fn_t> by_map;
  std::mutex mutex;
} by_fwd_t;


on_event_fn_t by_lock_(const by_keys_t & keys, fwd_new_stream_fn_t fwd_stream) {

  auto streams = std::make_shared<by_fwd_t>();

  return [=](e_t e) -> next_events_t
  {

     if (keys.empty()) {
       return {};
     }

     std::string key;
     for (const auto & k: keys) {
       key += event_str_value(e, k) + " ";
     }

     streams->mutex.lock();

     forward_fn_t & f = streams->by_map[key];

     if (!f) {

       f = fwd_stream();

     }

     streams->mutex.unlock();

     f({e});

     return {};

  };

}

streams_t by_lock(const by_keys_t & keys) {
  return create_stream(
      [=](fwd_new_stream_fn_t fwd_stream)
      {

        return by_lock_(keys, fwd_stream);

      });
};



typedef struct {
  std::unordered_map<std::string, Event> events;
  std::mutex mutex;
} coalesce_events_t;

on_event_fn_t coalesce_lock_(fold_fn_t fold) {

  auto coalesce = std::make_shared<coalesce_events_t>();

  return [=](e_t e)
    {

      std::vector<Event> events;
      std::vector<Event> expired_events;

      std::string key(e.host() + " " +  e.service());

      coalesce->mutex.lock();

      coalesce->events[key] = e;

      for (auto it = coalesce->events.begin(); it != coalesce->events.end();) {

        if (expired_(it->second)) {

          expired_events.push_back(it->second);
          it = coalesce->events.erase(it);

        } else {

          events.push_back(it->second);
          it++;

        }
      }

      coalesce->mutex.unlock();

      next_events_t next_events;

      if (!expired_events.empty()) {
        next_events.push_back(fold(expired_events));
      }

      if (!events.empty()) {
        next_events.push_back(fold(events));
      }

      return next_events;

    };

}

streams_t coalesce_lock(fold_fn_t fold) {
  return create_stream([=](){ return coalesce_lock_(fold); });
}

typedef struct project {
  project(size_t n) : events(n, boost::none) {}
  std::vector<boost::optional<Event>> events;
  std::mutex mutex;
} project_t;

on_event_fn_t project_lock_(const predicates_t predicates, fold_fn_t fold) {

  auto project_events = std::make_shared<project_t>(predicates.size());

  return [=](e_t e) -> next_events_t {

      int match_index = -1;
      for (size_t i = 0; i < predicates.size(); i++) {
        if (predicates[i](e)) {
          match_index = i;
          break;
        }
      }

      if (match_index == -1) {
        return {};
      }

      std::vector<Event> events;
      std::vector<Event> expired_events;

      {
        std::lock_guard<std::mutex> lock(project_events->mutex);

        for (int i = 0; i < static_cast<int>(predicates.size()); i++) {

          auto & curr_events = project_events->events;

          if (i == match_index) {

            curr_events[i] = e;
            events.push_back(e);

          } else {

            if (curr_events[i]) {

              if (expired_(*curr_events[i])) {

                expired_events.push_back(*curr_events[i]);
                curr_events[i].reset();

              } else {
                events.push_back(*curr_events[i]);
              }
            }

          }

        }

      }

      next_events_t next_events;

      if (!expired_events.empty()) {
        next_events.push_back(fold(expired_events));
      }

      if (!events.empty()) {
        next_events.push_back(fold(events));
      }

      return next_events;


    };

}

streams_t project_lock(const predicates_t predicates, fold_fn_t fold) {
  return create_stream([=](){ return project_lock_(predicates, fold); });
}


typedef struct {
  std::string state;
  std::mutex mutex;
} state_t;


on_event_fn_t changed_state_lock_(std::string initial) {
  auto prev = std::make_shared<state_t>();
  prev->state = initial;

  return [=](e_t e) -> next_events_t {

      {
        std::lock_guard<std::mutex> guard(prev->mutex);
        if (prev->state == e.state()) {
          return {};
        } else {
          prev->state = e.state();
        }
      }

      return {e};

    };
}

streams_t changed_state_lock(std::string initial) {
  return create_stream([=](){ return changed_state_lock_(initial); } );
}

typedef struct {
  std::list<Event> events;
  std::mutex mutex;
} moving_event_window_t;

on_event_fn_t moving_event_window_lock_(size_t n, fold_fn_t fold) {

  auto window = std::make_shared<moving_event_window_t>();

  return [=](e_t e)->next_events_t {

      std::list<Event> events;

      {
        std::lock_guard<std::mutex> guard(window->mutex);

        window->events.push_back(e);

        if (window->events.size() == (n + 1)) {
          window->events.pop_front();
        }

        events = window->events;
      }

      return {fold({begin(events), end(events)})};

    };
}

streams_t moving_event_window_lock(size_t n, fold_fn_t fold) {
  return create_stream([=](){ return moving_event_window_lock_(n, fold); });
}

typedef struct {
  std::list<Event> events;
  std::mutex mutex;
} fixed_event_window_t;

on_event_fn_t fixed_event_window_lock_(size_t n, fold_fn_t fold) {

  auto window = std::make_shared<fixed_event_window_t>();

  return [=](e_t e) -> next_events_t {

      std::list<Event> event_list;

      {

        std::lock_guard<std::mutex> guard(window->mutex);

        window->events = conj(window->events, e);

        if (window->events.size() == n) {

          event_list = window->events;
          window->events.clear();

        } else {

          return {};

        }

      }

      if (!event_list.empty()) {
        return {fold({begin(event_list), end(event_list)})};
      } else {
        return {};
      }
   };

}

streams_t fixed_event_window_lock(size_t n, fold_fn_t fold) {
  return create_stream([=](){ return fixed_event_window_lock_(n, fold); });
}

struct event_time_cmp
{
  bool operator() (const Event & lhs, const Event & rhs) const
  {
    return (lhs.time() > rhs.time());
  }
};

typedef struct {
  std::priority_queue<Event, std::vector<Event>, event_time_cmp> pq;
  time_t max{0};
  std::mutex mutex;
} moving_time_window_t;

on_event_fn_t moving_time_window_lock_(time_t dt, fold_fn_t fold) {

  auto window = std::make_shared<moving_time_window_t>();

  return [=](e_t e) -> next_events_t {

        if (!e.has_time()) {
          return {};
         }

         std::priority_queue<Event, std::vector<Event>, event_time_cmp> pq;

        {
          std::lock_guard<std::mutex> guard(window->mutex);

          if (e.time() > window->max) {
            window->max = e.time();
          }

          window->pq.push(e);

          if (window->max < dt) {

            pq = window->pq;

          } else {

            while (!window->pq.empty() && window->pq.top().time()
                   <= (window->max - dt))
            {
              window->pq.pop();
            }

          }

          pq = window->pq;
       }

       std::vector<Event> events;

       while (!pq.empty()) {
         events.push_back(pq.top());
         pq.pop();
       }

       return {fold(events)};

      };
}

streams_t moving_time_window_lock(time_t dt, fold_fn_t fold) {
  return create_stream([=](){ return moving_time_window_lock_(dt, fold); });
}

typedef struct {
  std::priority_queue<Event, std::vector<Event>, event_time_cmp> pq;
  time_t start{0};
  time_t max{0};
  bool started{false};
  std::mutex mutex;
} fixed_time_window_t;

on_event_fn_t fixed_time_window_lock_(time_t dt, fold_fn_t fold) {

  auto window = std::make_shared<fixed_time_window_t>();

  return [=](e_t e) -> next_events_t {

      std::vector<Event> flush;

       // Ignore event with no time
        if (!e.has_time()) {
          return {};
        }

       {
          std::lock_guard<std::mutex> guard(std::mutex);

          if (!window->started) {
            window->started = true;
            window->start = e.time();
            window->pq.push(e);
            window->max = e.time();
            return {};
          }

          // Too old
          if (e.time() < window->start) {
            return {};
          }

          if (e.time() > window->max) {
            window->max = e.time();
          }

          time_t next_interval = window->start - (window->start % dt) + dt;

          window->pq.push(e);

          if (window->max < next_interval) {
            return {};
          }

          // We can flush a window
          while (!window->pq.empty()
                 && window->pq.top().time() < next_interval)
          {
            flush.emplace_back(window->pq.top());
            window->pq.pop();
          }

          window->start = next_interval;
      }

      if (!flush.empty()) {
        return {fold(flush)};
      } else {
        return {};
      }
  };

}

streams_t fixed_time_window_lock(time_t dt, fold_fn_t fold) {
  return create_stream([=]() { return fixed_time_window_lock_(dt, fold); });
}


typedef struct {
  std::string state;
  std::vector<Event> buffer;
  time_t start{0};
  std::mutex mutex;
} stable_t;

on_event_fn_t stable_lock_(time_t dt) {

  auto stable = std::make_shared<stable_t>();

  return [=](e_t e)->next_events_t
    {

      std::vector<Event> flush;

      {
        std::lock_guard<std::mutex> guard(stable->mutex);

        if (stable->state != e.state()) {
          stable->start = e.time();
          stable->buffer.clear();
          stable->buffer.push_back(e);
          stable->state = e.state();
          return {};
        }

        if (e.time() < stable->start) {
          return {};
        }

        if (stable->start + dt > e.time()) {
          stable->buffer.push_back(e);
          return {};
        }

        if (!stable->buffer.empty()) {
          flush = std::move(stable->buffer);
          }
      }

      flush.push_back(e);

      return flush;

  };

}

streams_t stable_lock(time_t dt) {
  return create_stream([=](){ return stable_lock_(dt); });
}

typedef struct {
  size_t forwarded{0};
  time_t new_interval{0};
  std::mutex mutex;
} throttle_t;

on_event_fn_t throttle_lock_(size_t n, time_t dt) {

  auto throttled = std::make_shared<throttle_t>();

  return [=](e_t e) mutable -> next_events_t {

      {
        std::lock_guard<std::mutex> guard(throttled->mutex);

        if (throttled->new_interval < e.time()) {
          throttled->new_interval += e.time() + dt;
          throttled->forwarded = 0;
        }

        if (throttled->forwarded < n) {
          throttled->forwarded++;
        } else {
          return {};
        }

      }

      return  {e};

    };

}

streams_t throttle_lock(size_t n, time_t dt) {
  return create_stream([=]() { return throttle_lock_(n, dt); });
}

struct percentiles_data_t {
  percentiles_data_t(std::vector<double> p) : percentiles(p) {}
  std::vector<double> percentiles;
  reservoir reserv;
  Event event;
  std::atomic<bool> initialized{false};
};

void push_percentiles_(percentiles_data_t & percentiles, forward_fn_t forward) {

  if (!percentiles.initialized) {
    return;
  }

  forward(instrumentation::reservoir_to_events(percentiles.reserv,
                                               percentiles.percentiles,
                                               percentiles.event));
}

void update_percentiles_(e_t e, percentiles_data_t &  percentiles) {

  if (!metric_set(e)) {
    return;
  }

  percentiles.reserv.add_sample(metric_to_double(e));

  if (!percentiles.initialized.exchange(true)) {
    percentiles.event = e;
  }

}

streams_t percentiles_lock(time_t interval, std::vector<double> p) {

  return create_stream(

    // on_init_stream
    [=](fwd_new_stream_fn_t fwd_new_stream) -> on_event_fn_t
    {

      auto percentiles = std::make_shared<percentiles_data_t>(p);
      auto forward = fwd_new_stream();

      // Schedule periodic task to report percentiles
      g_core->sched().add_periodic_task(
        [=]() { push_percentiles_(*percentiles, forward); }, interval);

      // on_event_fn function that does that counts events
      return [=](e_t e) -> next_events_t
      {
         update_percentiles_(e, *percentiles);
         return {};
      };

    });
}

typedef struct {
  double metric{0};
  int64_t time{0};
  bool initialized{false};
  std::mutex mutex;
} ddt_prev_t;

on_event_fn_t ddt_lock_() {
  auto prev = std::make_shared<ddt_prev_t>();

  return [=](e_t e) -> next_events_t {

      double metric;

      {
        std::lock_guard<std::mutex> guard(std::mutex);

        auto tmp_metric = prev->metric;
        auto tmp_time = prev->time;

        prev->metric = metric_to_double(e);
        prev->time =  e.time();

        if (!prev->initialized) {
          prev->initialized = true;
          return {};
        }

        auto dt = e.time() - tmp_time;

        if (dt == 0) {
          return {};
        }

        metric = (prev->metric - tmp_metric) / dt;

      }

      Event ne(e);
      set_metric(ne, metric);
      return {ne};

    };
}

streams_t ddt_lock() {
  return create_stream(ddt_lock_);
}

