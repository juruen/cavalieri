#include <glog/logging.h>
#include <util.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <riemann_client/rieman_tcp_client_pool.h>

using namespace std::placeholders;

riemann_tcp_client_pool::riemann_tcp_client_pool(size_t thread_num,
                                                 const std::string host,
                                                 const int port)
  :
  tcp_client_pool_(
    thread_num,
    host,
    port,
    std::bind(&riemann_tcp_client_pool::output_event, this, _1)
  )
{
}

void riemann_tcp_client_pool::push_event(const Event & event) {
  tcp_client_pool_.push_event(event);
}

std::vector<char> riemann_tcp_client_pool::output_event(const Event & event) {

  std::vector<char> buffer;

  Msg msg;
  msg.set_ok(true);
  *msg.add_events() = event;

  auto nsize = htonl(msg.ByteSize());

  buffer.resize(sizeof(nsize) + msg.ByteSize());

  memcpy(&buffer[0], static_cast<void*>(&nsize), sizeof(nsize));
  msg.SerializeToArray(&buffer[0] + sizeof(nsize), msg.ByteSize());

  return buffer;
}
