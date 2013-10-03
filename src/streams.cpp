#include "streams.h"
#include <glog/logging.h>
#include <sstream>

static inline std::string metric_to_string(const Event& e) {
  std::ostringstream ss;
  if (e.has_metric_f()) {
    ss << e.metric_f();
  } else if (e.has_metric_d()) {
    ss << e.metric_d();
  } else if (e.has_metric_sint64()) {
    ss << e.metric_sint64();
  } else {
    ss << "nil";
  }
  return ss.str();
}

stream_t Streams::prn() {
  return [](const Event& e) {
    return;
    VLOG(3) << "prn()";
    LOG(INFO) << "prn() { host: '" <<  e.host() << "' service: '" << e.service()
              << "' description: '" << e.description()
              << "' state: '" << e.state() << "' metric: '"
              << metric_to_string(e) << "' }";
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
