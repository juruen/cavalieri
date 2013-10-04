#include "streams.h"
#include "util.h"
#include <glog/logging.h>


void call_rescue(e_t e, const children_t& children) {
  for (auto& s: children) {
    s(e);
  }
}

stream_t prn() {
  return [](e_t e) {
    VLOG(3) << "prn()";
    LOG(INFO) << "prn() { host: '" <<  e.host() << "' service: '" << e.service()
              << "' description: '" << e.description()
              << "' state: '" << e.state() << "' metric: '"
              << metric_to_string(e) << "' }";
  };
}

stream_t with(const with_changes_t& changes, const children_t& children) {
  return [=](e_t e) {
    Event ne(e);
    for (auto &kv: changes) {
      VLOG(3) << "with()  first: " << kv.first << " second: " << kv.second;
      std::string key = kv.first;
      if (key == "host") {
        ne.set_host(kv.second);
      } else if (key == "service") {
        ne.set_service(kv.second);
      } else if (key == "description") {
        ne.set_description(kv.second);
      } else if (key == "state") {
        ne.set_state(kv.second);
      } else if (key == "metric") {
        if (e.has_metric_f()) {
          ne.set_metric_f(atof(kv.second.c_str()));
        } else if (e.has_metric_d()) {
          ne.set_metric_d(atof(kv.second.c_str()));
        } else if (e.has_metric_sint64()) {
          ne.set_metric_sint64(atoi(kv.second.c_str()));
        }
      }
    }
    call_rescue(ne, children);
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
  double* rate = new double(0);

  CallbackTimer* callbackTimer = new CallbackTimer(interval,
      [=]()
      {
        VLOG(3) << "rate-timer()";
        Event e;
        e.set_metric_f(*rate / interval);
        *rate = 0;
        VLOG(3) << "rate-timer() value: " << e.metric_f();
        call_rescue(e, children);
      });

  (void)(callbackTimer);

  return [=](e_t e) {
    VLOG(3) << "rate() +1";
    *rate += metric_to_double(e);
  };
}

stream_t changed_state(std::string initial, const children_t& children) {
  std::string* last_state = new std::string(initial);

  return [=](e_t e) {
    VLOG(3) << "changed_state() last_state: "
            << last_state << " new state " << e.state();
    if (*last_state != e.state()) {
      VLOG(3) << "changed_state() state change";
      last_state->assign(e.state());
      call_rescue(e, children);
    }
  };
}

void Streams::add_stream(stream_t stream) {
  VLOG(3) << "adding stream";
  streams.push_back(stream);
}

void Streams::process_message(const Msg& message) {
  VLOG(3) << "process message. num of streams " << streams.size();
  for (int i = 0; i < message.events_size(); i++) {
    for (auto& s: streams) {
      s(message.events(i));
    }
  }
}
