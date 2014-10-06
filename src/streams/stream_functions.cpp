#include <glog/logging.h>
#include <boost/optional.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <util/util.h>
#include <core/core.h>
#include <scheduler/scheduler.h>
#include <predicates/predicates.h>
#include <rules_loader.h>
#include <streams/stream_functions_lock.h>
#include <streams/stream_functions.h>

namespace {

const size_t k_streams = 10;

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

namespace pred = predicates;

streams_t  prn() {
  return create_stream(
    [](e_t e) -> next_events_t
    {
      LOG(INFO) << "prn() " <<  e.json_str();

      return {e};
    });
}

streams_t  prn(const std::string prefix) {
  return create_stream(
    [=](e_t e) -> next_events_t
    {
      LOG(INFO) << "prn() " << prefix <<  e.json_str();

      return {e};
    });
}

streams_t null() {
  return create_stream([=](e_t) -> next_events_t { return {}; });
}

streams_t service(const std::string service) {
  return where(pred::match("service", service));
}

streams_t service_any(const std::vector<std::string> services) {
  return where(pred::match_any("service", services));
}

streams_t service_like(const std::string pattern) {
  return where(pred::match_like("service", pattern));
}

streams_t service_like_any(const std::vector<std::string> patterns) {
  return where(pred::match_like_any("service", patterns));
}

streams_t has_attribute(const std::string attribute) {
  return where(PRED(e.has_attr(attribute)));
}

streams_t state(const std::string state) {
  return where(pred::match("state", state));
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

streams_t set_service(const std::string service) {
  return WITH(e.set_service(service));
}

streams_t set_state(const std::string state) {
  return WITH(e.set_state(state));
}

streams_t set_host(const std::string host) {
  return WITH(e.set_host(host));
}

streams_t set_metric(const double metric) {
  return WITH(e.set_metric(metric));
}

streams_t set_description(const std::string description) {
  return WITH(e.set_description(description));
}

streams_t set_ttl(const float ttl) {
  return WITH(e.set_ttl(ttl));
}

streams_t default_service(const std::string service) {
  return WITH(e.has_service() ? e : e.set_service(service));
}

streams_t default_state(const std::string state) {
  return WITH(e.has_state() ? e : e.set_state(state));
}

streams_t default_host(const std::string host) {
  return WITH(e.has_host() ? e : e.set_host(host));
}

streams_t default_metric(const double metric) {
  return WITH(e.has_metric() ? e : e.set_metric(metric));
}

streams_t default_description(const std::string description) {
  return WITH(e.has_description() ? e : e.set_description(description));
}

streams_t default_ttl(const float ttl) {
  return WITH(e.has_ttl() ? e : e.set_ttl(ttl));
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


streams_t by(const by_keys_t & keys, const streams_t stream) {

#ifdef BY_LOCKFREE
  return by_lockfree(keys, stream);
#else
  return by_lock(keys, stream);
#endif

}

streams_t by(const by_keys_t & keys) {

#ifdef BY_LOCKFREE
  return by_lockfree(keys, stream);
#else
  return by_lock(keys);
#endif

}



streams_t rate(const int interval) {

  return create_stream(

    // on_init_stream
    [=](fwd_new_stream_fn_t fwd_new_stream) -> on_event_fn_t
    {

      auto rate = std::make_shared<std::atomic<double>>(0);
      auto forward = fwd_new_stream();

      // Schedule periodic task to report rate
      g_core->sched().add_periodic_task(
        [=]()
        {
          VLOG(1) << "rate-timer()";
          Event event;

          event.set_metric_d(rate->exchange(0) / interval);
          event.set_time(g_core->sched().unix_time());

          forward({event});
        },

        interval
        );

      // on_event_fn function that does that counts events
      return [=](e_t e) -> next_events_t
      {

        double expected, newval;

        do {

          expected = rate->load();
          newval = expected + e.metric();

        } while (!rate->compare_exchange_strong(expected, newval));

        return {};

      };

    });
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

  return by({"host", "service"}, changed_state_(initial));

}

streams_t changed_state() {

  return changed_state("ok");

}

streams_t tagged_any(const tags_t& tags) {
  return create_stream(
    [=](e_t e) -> next_events_t
    {

      if (pred::tagged_any(e, tags)) {
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
      if (pred::tagged_all(e, tags)) {
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

streams_t percentiles(time_t interval, std::vector<double> percentiles) {

#ifdef PERCENTILES_LOCKFREE
  return percentiles_lockfree(interval, percentiles);
#else
  return percentiles_lock(interval, percentiles);
#endif


}

streams_t above(double m) {
  return create_stream(
   [=](e_t e) -> next_events_t {

      if (pred::above(e, m)) {
        return {e};
      } else {
        return {};
      }

  });
}

streams_t under(double m) {
  return create_stream(
   [=](e_t e) -> next_events_t {

      if (pred::under(e, m)) {
        return {e};
      } else {
        return {};
      }

  });
}

streams_t within(double a, double b) {
  return create_stream(
    [=](e_t e) -> next_events_t {

    if (pred::above_eq(e,a) && pred::under_eq(e, b)) {
      return {e};
    } else {
      return {};
    }

  });
}

streams_t without(double a, double b) {
  return create_stream(
    [=](e_t e) -> next_events_t {

    if (pred::under(e,a) || pred::above(e, b)) {
      return {e};
    } else {
      return {};
    }

  });
}

streams_t scale(double s) {
  return create_stream(
    [=](e_t e) -> next_events_t {

      return {e.copy().set_metric(s * e.metric())};

  });
}

streams_t svec(std::vector<streams_t> streams) {
  return create_stream(
    [=]() mutable -> on_event_fn_t {

      for (streams_t & s : streams) {
        init_streams(s);
      }

      return [=](e_t e) -> next_events_t {

        if (!streams.empty()) {

          next_events_t next_events;

          for (const auto & stream : streams) {

            const auto ret = push_event(stream, e);
            std::copy(begin(ret), end(ret), back_inserter(next_events));

          }

          return next_events;

        } else {

          return {e};

        }

      };

    }

    );
}

on_event_fn_t counter_() {
   auto counter = std::make_shared<std::atomic<unsigned int>>(0);

   return [=](e_t e) -> next_events_t {

      if (e.has_metric()) {

        return {e.copy().clear_metric()
                .set_metric_sint64(counter->fetch_add(1) + 1)};

      } else {

        return {e};

      }
    };
}

streams_t counter() {
  return create_stream(counter_);
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
      if (pred::expired(e)) {
        return {e};
      } else {
        return {};
      }
  });
}

streams_t not_expired() {
  return create_stream(
    [=](e_t e) -> next_events_t {
      if (pred::expired(e)) {
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
        ne.add_tag(t);
      }

      return {ne};
    }
  );
}

streams_t send_index() {
  return create_stream(
    [=](e_t e) -> next_events_t {

      if (!pred::expired(e)) {

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
                const std::string to)
{

  return email(server, from, std::vector<std::string>{to});

}

streams_t email(const std::string server, const std::string from,
                const std::vector<std::string> to) {
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

streams::streams(const config & conf, instrumentation & instrumentation)
  : rules_directory_(conf.rules_directory),
    streams_(k_streams),
    instrumentation_(instrumentation),
    stop_(false)
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

void streams::add_stream(streams_t) {
  VLOG(3) << "add_stream()";
  //streams_.push_back(stream);
}

void streams::reload_rules() {
 load_rules(rules_directory_, streams_);
}

void streams::process_message(const riemann::Msg& message) {

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

void push_stream(stream_lib & stream, const Event & event) {

  if (!stream.inc()) {
    return;
  }

  set_thread_ns(stream.file);

  try {
    ::push_event(*stream.stream, event);
  } catch(const std::exception & e){
    LOG(ERROR) << "exception in " << stream.file << " : "  << e.what();
  }

  set_thread_global_ns();

  stream.dec();
}

void streams::push_event(const Event& e) {

  if (stop_) {
    return;
  }

  for (auto & s: streams_) {

    if (!s.used()) {
      continue;
    }

    if (e.state() == "expired") {
      ::push_stream(s, e);
      continue;
    }

    if (e.has_time()) {

      update_in_latency(instrumentation_, in_latency_id_, e.time());

      auto start_time = now();

      ::push_stream(s, e);

      update_latency(instrumentation_, latency_id_, start_time);

    } else {
      Event ne(e);
      ne.set_time(g_core->sched().unix_time());

      ::push_stream(s, ne);
    }

  }


}

void streams::stop() {

  VLOG(3) << "stop stream processing";

  stop_ = true;
}
