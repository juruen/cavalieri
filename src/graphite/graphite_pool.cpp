#include <glog/logging.h>
#include <util.h>
#include <stdio.h>
#include <graphite/graphite_pool.h>

using namespace std::placeholders;

namespace {

const size_t k_buffer_size = 2048;

}

graphite_pool::graphite_pool(size_t thread_num, const std::string host,
                             const int port)
  :
  tcp_client_pool_(
    thread_num,
    host,
    port,
    std::bind(&graphite_pool::output_event, this, _1)
  )
{
}

void graphite_pool::push_event(const Event & event) {
  tcp_client_pool_.push_event(event);
}

std::vector<char> graphite_pool::output_event(const Event & event) {

  char buffer[k_buffer_size];

  auto written = snprintf(buffer, k_buffer_size, "%s.%s %f %lld\n",
                          event.host().c_str(),
                          event.service().c_str(),
                          metric_to_double(event),
                          event.time());

  return std::vector<char>(buffer, buffer + written);

}
