#include <glog/logging.h>
#include <boost/optional.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <atom/atom.h>
#include <util.h>
#include <core/core.h>
#include <scheduler/scheduler.h>
#include <streams/stream_functions_lock.h>
#include <streams/stream_functions.h>

namespace {

const unsigned int k_default_ttl = 60;

const std::string k_rate_service = "cavalieri stream rate";
const std::string k_rate_desc = "events per second in streams";

const std::string k_latency_service = "cavalieri stream latency";
const std::string k_latency_desc = "distribution of proccessing time "
                                   "of events through streams";

const std::string k_in_latency_service = "cavalieri incoming events "
                                         "latency";

const std::string k_in_latency_desc = "distribution of processing time "
                                      "before entering streams";



const std::vector<double> k_percentiles = {0.0, .5, .95, .99, 1};

using namespace std::chrono;
using time_point_t = high_resolution_clock::time_point;

time_point_t now() {

  return std::chrono::high_resolution_clock::now();

}

void update_latency(instrumentation & inst, const int id, time_point_t start)
{

  inst.update_latency(
      id,
      duration_cast<microseconds>(now() - start).count() / 1000.0
  );

}

void update_in_latency(instrumentation & inst, const int id,
                       const long int start)
{

  inst.update_latency(id, difftime(time(0), start));

}



}

streams_t  prn() {
  return create_stream(
    [](e_t e) -> next_events_t
    {
      LOG(INFO) << "prn() " <<  event_to_json(e);

      return {};
    });
}

streams_t  prn(const std::string prefix) {
  return create_stream(
    [=](e_t e) -> next_events_t
    {
      LOG(INFO) << "prn() " << prefix <<  event_to_json(e);

      return {};
    });
}

streams_t service(const std::string service) {
  return where(match_pred("service", service));
}

streams_t service_any(const std::vector<std::string> services) {
  return where(match_any_pred("service", services));
}

streams_t service_like(const std::string pattern) {
  return where(match_like_pred("service", pattern));
}

streams_t service_like_any(const std::vector<std::string> patterns) {
  return where(match_like_any_pred("service", patterns));
}

streams_t state(const std::string state) {
  return where(match_pred("state", state));
}

streams_t state_any(const std::vector<std::string> states) {
  return create_stream(
    [=](e_t e) -> next_events_t
    {
      if (std::find(begin(states), end(states), e.state()) != end(states)) {
        return {e};
      } else {
        return {};
      }
    });
}

streams_t set_state(const std::string state) {
  return with({{"state", state}});
}

streams_t set_metric(const double metric) {
  return with({{"metric", metric}});
}

streams_t with(const with_changes_t & changes, const bool & replace)
{
  return create_stream(
    [=](e_t e) -> next_events_t
    {
      std::vector<Event> ne = {e};

      for (auto & kv: changes) {
        set_event_value(ne[0], kv.first, kv.second, replace);
      }

      return ne;

    });
}

streams_t with(const with_changes_t& changes) {
  return with(changes, true);
}

streams_t default_to(const with_changes_t& changes)
{
  return with(changes, false);
}

streams_t split_(const split_clauses_t clauses, streams_t default_stream)
{
  return create_stream(

    [=](e_t e) -> next_events_t {

      for (auto const & pair: clauses) {

        if (pair.first(e)) {

          return push_event(pair.second, e);

        }

      }

      if (!default_stream.empty()) {

        return push_event(default_stream, e);

      } else {

        return {e};
      }

  });
}

streams_t split(const split_clauses_t clauses)
{
  return split_(clauses, {});
}

streams_t split(const split_clauses_t clauses, streams_t default_stream)
{
  return split_(clauses, default_stream);
}

streams_t where(const predicate_t & predicate)
{
  return create_stream(
    [=](e_t e) -> next_events_t {

      if (predicate(e)) {
        return {e};
      } else {
        return {};
      }

  });
}

streams_t where(const predicate_t & predicate,
                   streams_t else_stream)
{
  return create_stream(
    [=](e_t e) -> next_events_t {

      if (predicate(e)) {
        return {e};
      } else {
        return push_event(else_stream, e);
      }

  });
}


streams_t by(const by_keys_t & keys, const by_stream_t stream) {

#ifdef BY_LOCKFREE
  return by_lockfree(keys, stream);
#else
  return by_lock(keys, stream);
#endif

}

streams_t rate(const int interval) {

  auto rate = std::make_shared<std::atomic<double>>(0);

  return create_stream(

    [=](e_t e) mutable ->next_events_t
    {

      double expected, newval;

      do {

        expected = rate->load();
        newval = expected + metric_to_double(e);

      } while (!rate->compare_exchange_strong(expected, newval));

      return {};

    });
  /*
    [=](forward_fn_t forward)
    {

      g_core->sched().add_periodic_task(
          [=]() mutable
          {

            VLOG(3) << "rate-timer()";

            Event event;

            event.set_metric_d(rate->exchange(0) / interval);
            event.set_time(g_core->sched().unix_time());

            forward(event);
          },

          interval
          );

    }

  );
  */

}

streams_t coalesce(fold_fn_t fold) {

#ifdef COALESCE_LOCKFREE
  return coalesce_lockfree(fold);
#else
  return coalesce_lock(fold);
#endif

}

streams_t project(const predicates_t predicates, fold_fn_t fold) {

#ifdef PROJECT_LOCKFREE
  return project_lockfree(predicates, fold);
#else
  return project_lock(predicates, fold);
#endif

}

streams_t changed_state_(std::string initial) {

#ifdef CHANGED_STATE_LOCKFREE
  return changed_state_lockfree(initial);
#else
  return changed_state_lock(initial);
#endif

}

streams_t changed_state(std::string initial) {

  return by({"host", "service"}, BY(changed_state_(initial)));

}

streams_t tagged_any(const tags_t& tags) {
  return create_stream(
    [=](e_t e) -> next_events_t
    {

      if (tagged_any_(e, tags)) {
        return {e};
      } else {
        return {};
      }

    });
}

streams_t tagged_all(const tags_t& tags) {
  return create_stream(
    [=](e_t e) -> next_events_t
    {
      if (tagged_all_(e, tags)) {
        return {e};
      } else {
        return {};
      }

    });
}

streams_t tagged(const std::string tag) {
  return tagged_all({tag});
}

streams_t smap(smap_fn_t f) {
  return create_stream(
    [=](e_t e) -> next_events_t
    {
      Event ne(e);

      f(ne);

      return {ne};
    });
}

streams_t moving_event_window(size_t n, fold_fn_t fold) {

#ifdef MOVING_EVENT_WINDOW_LOCKFREE
  return moving_event_window_lockfree(n, fold);
#else
  return moving_event_window_lock(n, fold);
#endif

}

streams_t fixed_event_window(size_t n, fold_fn_t fold) {

#ifdef FIXED_EVENT_WINDOW_LOCKFREE
  return fixed_event_window_lockfree(n, fold);
#else
  return fixed_event_window_lock(n, fold);
#endif

}

streams_t moving_time_window(time_t dt, fold_fn_t fold) {

#ifdef MOVING_TIME_WINDOW_LOCKFREE
  return moving_time_window_lockfree(dt, fold);
#else
  return moving_time_window_lock(dt, fold);
#endif

}

streams_t fixed_time_window(time_t dt, fold_fn_t fold) {

#ifdef FIXED_TIME_WINDOW_LOCKFREE
  return fixed_time_window_lockfree(dt, fold);
#else
  return fixed_time_window_lock(dt, fold);
#endif

}

streams_t stable(time_t dt) {

#ifdef STABLE_LOCKFREE
  return stable_lockfree(dt);
#else
  return stable_lock(dt);
#endif

}

streams_t throttle(size_t n, time_t dt) {

#ifdef THROTTLE_LOCKFREE
  return throttle_lockfree(n, dt);
#else
  return throttle_lock(n, dt);
#endif

}

streams_t above(double m) {
  return create_stream(
   [=](e_t e) -> next_events_t {

      if (above_(e, m)) {
        return {e};
      } else {
        return {};
      }

  });
}

streams_t under(double m) {
  return create_stream(
   [=](e_t e) -> next_events_t {

      if (under_(e, m)) {
        return {e};
      } else {
        return {};
      }

  });
}

streams_t within(double a, double b) {
  return create_stream(
    [=](e_t e) -> next_events_t {

    if (above_eq_(e,a) && under_eq_(e, b)) {
      return {e};
    } else {
      return {};
    }

  });
}

streams_t without(double a, double b) {
  return create_stream(
    [=](e_t e) -> next_events_t {

    if (under_(e,a) || above_(e, b)) {
      return {e};
    } else {
      return {};
    }

  });
}

streams_t scale(double s) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      return {set_metric_c(e, s * metric_to_double(e))};

  });
}

streams_t svec(std::vector<streams_t> streams) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      if (!streams.empty()) {

        for (const auto & stream : streams) {

          push_event(stream, e);

        }

        return {};

      } else {

        return {e};

      }

    }

    );
}

streams_t counter() {
   auto counter = std::make_shared<std::atomic<unsigned int>>(0);

  return create_stream(
    [=](e_t e) mutable -> next_events_t {

      if (metric_set(e)) {

        Event ne(e);
        ne.set_metric_sint64(counter->fetch_add(1) + 1);
        return {ne};

      } else {

        return {e};

      }
  });
}

streams_t ddt() {

#ifdef DDT_LOCKFREE
  return ddt_lockfree();
#else
  return ddt_lock();
#endif


}

streams_t expired() {
  return create_stream(
    [=](e_t e) -> next_events_t {
      if (expired_(e)) {
        return {e};
      } else {
        return {};
      }
  });
}

streams_t tag(tags_t tags) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      Event ne(e);

      for (const auto & t: tags) {
        *(ne.add_tags()) = t;
      }

      return {ne};
    }
  );
}

streams_t send_index() {
  return create_stream(
    [=](e_t e) -> next_events_t {

      if (!expired_(e)) {

        g_core->idx().add_event(e);

      }

      return {};

    }
  );

}

streams_t send_graphite(const std::string host, const int port) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      g_core->externals().graphite(host, port, e);

      return {};

    }
  );
}

streams_t forward(const std::string host, const int port) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      g_core->externals().forward(host, port, e);

      return {};

    }
  );
}

streams_t email(const std::string server, const std::string from,
                const std::string to) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      g_core->externals().email(server, from, to, e);

      return {};

    }
  );
}

streams_t pagerduty_resolve(const std::string key) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      g_core->externals().pager_duty_resolve(key, e);

      return {};

    }
  );
}

streams_t pagerduty_acknowledge(const std::string key) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      g_core->externals().pager_duty_acknowledge(key, e);

      return {};

    }
  );
}

streams_t pagerduty_trigger(const std::string key) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      g_core->externals().pager_duty_trigger(key, e);

      return {};

    }
  );
}

predicate_t above_eq_pred(const double value) {
  return PRED(above_eq_(e, value));
}

predicate_t above_pred(const double value) {
  return PRED(above_(e, value));
}

predicate_t under_eq_pred(const double value) {
  return PRED(under_eq_(e, value));
}

predicate_t under_pred(const double value) {
  return PRED(under_(e, value));
}

predicate_t state_pred(const std::string state) {
  return PRED(e.state() == state);
}

predicate_t service_pred(const std::string service) {
  return PRED(e.service() == service);
}

predicate_t match_pred(const std::string key, const std::string value) {
  return PRED(match_(e, key, value));
}

predicate_t match_any_pred(const std::string key,
                           const std::vector<std::string> values)
{
  return PRED(
      std::find(values.begin(), values.end(), event_str_value(e, key))
      != values.end()
  );
}

predicate_t match_re_pred(const std::string key, const std::string value) {
  return PRED(match_re_(e, key, value));
}

predicate_t match_re_any_pred(const std::string key,
                              const std::vector<std::string> values)
{
  return [=](e_t e) {

    for (const auto & val : values) {
      if (match_re_(e, key, val)) {
        return true;
      }
    }

    return false;
  };
}

predicate_t match_like_pred(const std::string key, const std::string value) {
  return PRED(match_like_(e, key, value));
}

predicate_t match_like_any_pred(const std::string key,
                              const std::vector<std::string> values)
{
  return [=](e_t e) {

    for (const auto & val : values) {
      if (match_like_(e, key, val)) {
        return true;
      }
    }

    return false;
  };
}

predicate_t default_pred() {
  return [](e_t){ return true; };
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

bool expired_(e_t e) {
  auto ttl = e.has_ttl() ? e.ttl() : k_default_ttl;

  if (e.state() == "expired") {
    return true;
  }

  if (g_core->sched().unix_time() < e.time()) {
    return false;
  }

  return (g_core->sched().unix_time() - e.time() > ttl);
}

bool above_eq_(e_t e, const double value) {
  return (metric_to_double(e) >= value);
}

bool above_(e_t e, const double value) {
  return (metric_to_double(e) > value);
}

bool under_eq_(e_t e, const double value) {
  return (metric_to_double(e) <= value);
}

bool under_(e_t e, const double value) {
  return (metric_to_double(e) < value);
}

bool match_(e_t e, const std::string key, const std::string value) {

  const std::string ev_val(event_str_value(e, key));

  return ev_val == value;

}

bool match_re_(e_t e, const std::string key, const std::string value) {

  const std::string ev_val(event_str_value(e, key));

  return match_regex(ev_val, value);

}

bool match_like_(e_t e, const std::string key, const std::string value) {

  const std::string ev_val(event_str_value(e, key));

  return match_like(ev_val, value);

}

streams::streams(instrumentation & instrumentation)
  : instrumentation_(instrumentation), stop_(false)
{

  rate_id_ = instrumentation_.add_rate(k_rate_service,
                                       k_latency_desc);

  latency_id_ = instrumentation_.add_latency(k_latency_service,
                                             k_latency_desc,
                                             k_percentiles);

  in_latency_id_ = instrumentation_.add_latency(k_in_latency_service,
                                                k_in_latency_desc,
                                                k_percentiles);
}

void streams::add_stream(streams_t stream) {
  streams_.push_back(stream);
}

void streams::process_message(const Msg& message) {

  if (stop_) {
    return;
  }

  instrumentation_.update_rate(rate_id_, message.events_size());

  VLOG(3) << "process message. num of streams " << streams_.size();
  VLOG(3) << "process message. num of events " << message.events_size();

  for (int i = 0; i < message.events_size(); i++) {

    const Event & event = message.events(i);

    push_event(event);

  }
}

void streams::push_event(const Event& e) {

  if (stop_) {
    return;
  }

  for (const auto & s: streams_) {

    if (e.state() == "expired") {
      ::push_event(s, e);
      continue;
    }

    if (e.has_time()) {

      update_in_latency(instrumentation_, in_latency_id_, e.time());

      auto start_time = now();

      ::push_event(s, e);

      update_latency(instrumentation_, latency_id_, start_time);

    } else {
      Event ne(e);
      ne.set_time(g_core->sched().unix_time());

      ::push_event(s, ne);
    }

  }


}

void streams::stop() {

  VLOG(3) << "stop stream processing";

  stop_ = true;
}
