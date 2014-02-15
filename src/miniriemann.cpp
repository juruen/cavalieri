#include <ev++.h>
#include <glog/logging.h>
#include <iostream>
#include <thread>
#include "async_loop.h"
#include "real_index.h"
#include "tcpserver.h"
#include "riemanntcpconnection.h"
#include "websocket.h"
#include "streams.h"
#include "util.h"
#include "pubsub.h"
#include "driver.h"
#include "pagerduty.h"
#include "wsutil.h"
#include "incomingevents.h"
#include "riemann_tcp_pool.h"
#include "websocket_pool.h"
#include "scheduler.h"
#include "atom.h"


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

    riemann_tcp_pool rieman_tcp{1, [&](std::vector<unsigned char> e)
                                        {
                                          incoming.add_undecoded_msg(e);
                                        }};

    g_main_loop.add_tcp_listen_fd(create_tcp_listen_socket(5555),
                                [&](int fd) { rieman_tcp.add_client(fd); });

    websocket_pool ws(1, pubsub);
    g_main_loop.add_tcp_listen_fd(create_tcp_listen_socket(5556),
                                [&](int fd) {ws.add_client(fd); });


    g_main_loop.start();
  }

  cds::Terminate();

  return 0;
}
