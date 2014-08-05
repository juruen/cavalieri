#include <util.h>
#include <queue>
#include <streams/stream_functions.h>


typedef struct {
  std::unordered_map<std::string, streams_t> by_map;
  std::mutex mutex;
} by_t;


streams_t by_lock(const by_keys_t & keys, const by_stream_t stream) {

  auto streams = std::make_shared<by_t>();

  return create_stream(

    [=](e_t e)->next_events_t {

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

        s = stream();

      }

      streams->mutex.unlock();

      return push_event(s, e);

     });

}

typedef struct {
  std::unordered_map<std::string, Event> events;
  std::mutex mutex;
} coalesce_events_t;

streams_t coalesce_lock(fold_fn_t fold) {

  auto coalesce = std::make_shared<coalesce_events_t>();

  return create_stream(
    [=](e_t e) mutable
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

    }
  );

}

typedef struct project {
  project(size_t n) : events(n, boost::none) {}
  std::vector<boost::optional<Event>> events;
  std::mutex mutex;
} project_t;

streams_t project_lock(const predicates_t predicates, fold_fn_t fold) {

  auto project_events = std::make_shared<project_t>(predicates.size());

  return create_stream(

    [=](e_t e) mutable ->next_events_t {

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


    });

}

typedef struct {
  std::string state;
  std::mutex mutex;
} state_t;


streams_t changed_state_lock(std::string initial) {
  auto prev = std::make_shared<state_t>();
  prev->state = initial;

  return create_stream(
    [=](e_t e) -> next_events_t {

      {
        std::lock_guard<std::mutex> guard(prev->mutex);
        if (prev->state == e.state()) {
          return {};
        } else {
          prev->state = e.state();
        }
      }

      return {e};

    });
}

typedef struct {
  std::list<Event> events;
  std::mutex mutex;
} moving_event_window_t;

streams_t moving_event_window_lock(size_t n, fold_fn_t fold) {

  auto window = std::make_shared<moving_event_window_t>();

  return create_stream(
    [=](e_t e)->next_events_t {

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

    }
   );
}

typedef struct {
  std::list<Event> events;
  std::mutex mutex;
} fixed_event_window_t;

streams_t fixed_event_window_lock(size_t n, fold_fn_t fold) {

  auto window = std::make_shared<fixed_event_window_t>();

  return create_stream(

    [=](e_t e)->next_events_t {

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
   }

  );

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

streams_t moving_time_window_lock(time_t dt, fold_fn_t fold) {

  auto window = std::make_shared<moving_time_window_t>();

  return create_stream(

    [=](e_t e) -> next_events_t {

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

      }
    );
}

typedef struct {
  std::priority_queue<Event, std::vector<Event>, event_time_cmp> pq;
  time_t start{0};
  time_t max{0};
  bool started{false};
  std::mutex mutex;
} fixed_time_window_t;

streams_t fixed_time_window_lock(time_t dt, fold_fn_t fold) {

  auto window = std::make_shared<fixed_time_window_t>();

  return create_stream(

    [=](e_t e) -> next_events_t {

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
  });

}


typedef struct {
  std::string state;
  std::vector<Event> buffer;
  time_t start{0};
  std::mutex mutex;
} stable_t;

streams_t stable_lock(time_t dt) {

  auto stable = std::make_shared<stable_t>();

  return create_stream(

    [=](e_t e)->next_events_t
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

  });

}

typedef struct {
  size_t forwarded{0};
  time_t new_interval{0};
  std::mutex mutex;
} throttle_t;

streams_t throttle_lock(size_t n, time_t dt) {

  auto throttled = std::make_shared<throttle_t>();

  return create_stream(
    [=](e_t e) mutable -> next_events_t {

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

    });

}

typedef struct {
  double metric{0};
  int64_t time{0};
  bool initialized{false};
  std::mutex mutex;
} ddt_prev_t;

streams_t ddt_lock() {
  auto prev = std::make_shared<ddt_prev_t>();

  return create_stream(
    [=](e_t e) -> next_events_t {

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

    });
}


