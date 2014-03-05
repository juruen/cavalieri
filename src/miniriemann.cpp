#include <glog/logging.h>
#include <async/async_loop.h>
#include <index/real_index.h>
#include <transport/listen_tcp_socket.h>
#include <riemann_tcp_pool.h>
#include <websocket_pool.h>
#include <streams.h>
#include <util.h>
#include <pubsub.h>
#include <query/driver.h>
#include "pagerduty.h"
#include <incoming_events.h>
#include <riemann_tcp_pool.h>
#include <riemann_udp_pool.h>
#include <scheduler/scheduler.h>
#include <atom.h>


int main(int, char **argv)
{
  cds::Initialize();
  {
    cds::gc::HP hpGC;
    atom<bool>::attach_thread();

    google::InitGoogleLogging(argv[0]);

    pub_sub pubsub;
    real_index real_idx(pubsub, [](e_t){}, 60);
    class index idx(real_idx);

    streams streams;
    streams.add_stream(send_index(idx));

    incoming_events incoming(streams);

    riemann_tcp_pool rieman_tcp{1, [&](const std::vector<unsigned char> e)
                                        {
                                          incoming.add_undecoded_msg(e);
                                        }};

    g_main_loop.add_tcp_listen_fd(create_tcp_listen_socket(5555),
                                [&](int fd) { rieman_tcp.add_client(fd); });

    riemann_udp_pool rieman_udp{[&](const std::vector<unsigned char> e)
                                    {
                                      incoming.add_undecoded_msg(e);
                                    }};

    websocket_pool ws(1, pubsub);
    g_main_loop.add_tcp_listen_fd(create_tcp_listen_socket(5556),
                                [&](int fd) {ws.add_client(fd); });


    g_main_loop.start();
  }

  cds::Terminate();

  return 0;
}
