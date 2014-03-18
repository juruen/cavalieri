#include <glog/logging.h>
#include <transport/listen_tcp_socket.h>
#include <core.h>

namespace {

const uint64_t k_expire_period = 60;

void detach_thread(std::function<void()> fn) {
  std::thread t(fn);
  t.detach();
}

std::shared_ptr<class index> init_index(std::shared_ptr<pub_sub> pubsub,
                                        std::shared_ptr<streams> streams)
{
  auto push_event = [=](e_t e) { streams->push_event(e); };

  return std::make_shared<class index>(create_index(*pubsub, push_event,
                                                    k_expire_period,
                                                    detach_thread));
}

inline void incoming_event(const std::vector<unsigned char> raw_msg,
                           std::shared_ptr<streams> streams)
{
    Msg msg;

    if (!msg.ParseFromArray(&raw_msg[0], raw_msg.size())) {
      VLOG(2) << "error parsing protobuf payload";
      return;
    }

    streams->process_message(msg);
}

std::shared_ptr<riemann_tcp_pool> init_tcp_server(
    std::shared_ptr<main_async_loop> loop,
    std::shared_ptr<streams> streams)
{

  auto income_tcp_event = [=](const std::vector<unsigned char> raw_msg)
  {
    incoming_event(raw_msg, streams);
  };

  auto tcp_server = std::make_shared<riemann_tcp_pool>(1, income_tcp_event);

  loop->add_tcp_listen_fd(create_tcp_listen_socket(5555),
                         [=](int fd) {tcp_server->add_client(fd); });

  return tcp_server;
}

std::shared_ptr<riemann_udp_pool> init_udp_server(
    std::shared_ptr<streams> streams)
{

  auto income_udp_event = [=](const std::vector<unsigned char> raw_msg)
  {
    incoming_event(raw_msg, streams);
  };

  auto udp_server = std::make_shared<riemann_udp_pool>(income_udp_event);

  return udp_server;
}

std::shared_ptr<websocket_pool> init_ws_server(
    std::shared_ptr<main_async_loop> loop,
    std::shared_ptr<pub_sub> pubsub)
{

  auto ws_server = std::make_shared<websocket_pool>(1, *pubsub);

  loop->add_tcp_listen_fd(create_tcp_listen_socket(5556),
                         [=](int fd) {ws_server->add_client(fd); });

  return ws_server;
}


}

core::core()
  :
    main_loop_(new main_async_loop(create_main_async_loop())),

    streams_(new streams()),

    pubsub_(new pub_sub()),

    index_(init_index(pubsub_, streams_)),

    tcp_server_(init_tcp_server(main_loop_, streams_)),

    udp_server_(init_udp_server(streams_)),

    ws_server_(init_ws_server(main_loop_, pubsub_))
{

}

void core::start() {
  main_loop_->start();
}
