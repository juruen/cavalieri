#include <glog/logging.h>
#include <util/util.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <proto.pb.h>
#include <external/rieman_tcp_client_pool.h>

namespace {

size_t  k_default_batch_size = 100;

}

using namespace std::placeholders;

riemann_tcp_client_pool::riemann_tcp_client_pool(size_t thread_num,
                                                 const std::string host,
                                                 const int port)
  :
  tcp_client_pool_(
    thread_num,
    host,
    port,
    k_default_batch_size,
    std::bind(&riemann_tcp_client_pool::output_events, this, _1)
  )
{
}

void riemann_tcp_client_pool::push_event(const Event & event) {
  tcp_client_pool_.push_event(event);
}

std::vector<char> riemann_tcp_client_pool::output_events(
    const std::vector<Event> events)
{

  std::vector<char> buffer;

  riemann::Msg msg;
  msg.set_ok(true);

  for (const auto & event : events) {
   *msg.add_events() = event.riemann_event();
  }

  auto nsize = htonl(msg.ByteSize());

  buffer.resize(sizeof(nsize) + msg.ByteSize());

  memcpy(&buffer[0], static_cast<void*>(&nsize), sizeof(nsize));
  msg.SerializeToArray(&buffer[0] + sizeof(nsize), msg.ByteSize());

  return buffer;
}
