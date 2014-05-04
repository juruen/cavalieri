#include <glog/logging.h>
#include <util.h>
#include <stdio.h>
#include <external/graphite_pool.h>

using namespace std::placeholders;

namespace {

const size_t k_buffer_size = 2048;
const size_t k_batch_size = 100;

}

graphite_pool::graphite_pool(size_t thread_num, const std::string host,
                             const int port)
  :
  tcp_client_pool_(
    thread_num,
    host,
    port,
    k_batch_size,
    std::bind(&graphite_pool::output_events, this, _1)
  )
{
}

void graphite_pool::push_event(const Event & event) {
  tcp_client_pool_.push_event(event);
}

std::vector<char> graphite_pool::output_events(const std::vector<Event> events)
{

  // TODO Use a single buffer to reduce the number of copies
  std::vector<char> output;

  for (const auto & event : events) {

    std::vector<char> buffer;
    buffer.reserve(k_buffer_size);

    auto written = snprintf(&buffer[0], buffer.capacity(), "%s.%s %f %lld\n",
                            event.host().c_str(),
                            event.service().c_str(),
                            metric_to_double(event),
                            event.time());

    buffer.resize(written);

    output.reserve(output.capacity() + buffer.size());
    output.insert(output.end(), buffer.begin(), buffer.end());
  }

  return output;

}
