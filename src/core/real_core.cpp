#include <glog/logging.h>
#include <transport/listen_tcp_socket.h>
#include <rules_loader.h>
#include <core/real_core.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>
#include <unistd.h>

namespace {

void detach_thread(std::function<void()> fn) {
  std::thread t(fn);
  t.detach();
}

std::shared_ptr<class index> init_index(const config & conf,
                                        std::shared_ptr<pub_sub> pubsub,
                                        std::shared_ptr<streams> streams)
{
  auto push_event = [=](e_t e) { streams->push_event(e); };

  return std::make_shared<class index>(create_index(*pubsub, push_event,
                                                    conf.index_expire_interval,
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
    const config & conf,
    std::shared_ptr<main_async_loop> loop,
    std::shared_ptr<streams> streams)
{

  auto income_tcp_event = [=](const std::vector<unsigned char> raw_msg)
  {
    incoming_event(raw_msg, streams);
  };

  auto tcp_server = std::make_shared<riemann_tcp_pool>(
      conf.riemann_tcp_pool_size,
      income_tcp_event
  );

  loop->add_tcp_listen_fd(create_tcp_listen_socket(conf.events_port),
                         [=](int fd) {tcp_server->add_client(fd); });

  return tcp_server;
}

std::shared_ptr<riemann_udp_pool> init_udp_server(
    const config & conf,
    std::shared_ptr<streams> streams)
{

  auto income_udp_event = [=](const std::vector<unsigned char> raw_msg)
  {
    incoming_event(raw_msg, streams);
  };

  auto udp_server = std::make_shared<riemann_udp_pool>(conf.events_port,
                                                       income_udp_event);

  return udp_server;
}

std::shared_ptr<websocket_pool> init_ws_server(
    const config & conf,
    std::shared_ptr<main_async_loop> loop,
    std::shared_ptr<pub_sub> pubsub)
{

  auto ws_server = std::make_shared<websocket_pool>(conf.ws_pool_size, *pubsub);

  loop->add_tcp_listen_fd(create_tcp_listen_socket(conf.ws_port),
                         [=](int fd) {ws_server->add_client(fd); });

  return ws_server;
}

}

real_core::real_core(const config & conf)
  :
    config_(conf),

    main_loop_(new main_async_loop(create_main_async_loop())),

    streams_(new streams()),

    pubsub_(new pub_sub()),

    index_(init_index(conf, pubsub_, streams_)),

    tcp_server_(init_tcp_server(conf, main_loop_, streams_)),

    udp_server_(init_udp_server(conf, streams_)),

    ws_server_(init_ws_server(conf, main_loop_, pubsub_)),

    graphite_(new graphite()),

    riemann_client_(new riemann_tcp_client())
{

}

void real_core::start() {

  for (const auto & stream : load_rules(config_.rules_directory)) {
    add_stream(stream);
  }

  LOG(INFO) << "Brace for impact, starting nuclear core.";

  main_loop_->start();

  LOG(INFO) << "Screw you guys, I'm going home.";
}

void real_core::add_stream(std::shared_ptr<streams_t> stream) {

  if (stream) {

    sh_streams_.push_back(stream);

    streams_->add_stream(*stream);

  }

}

std::shared_ptr<class index> real_core::index() {
  return index_;
}

void real_core::send_to_graphite(const std::string host, const int port,
                                 const Event & event)
{
  graphite_->push_event(host, port, event);
}

void real_core::forward(const std::string host, const int port,
                        const Event & event)
{
  riemann_client_->push_event(host, port, event);
}

void start_core(int argc, char **argv) {

  int orig_argc = argc;
  char **orig_argv = copy_args(argc, argv);

  FLAGS_logtostderr = true;
  FLAGS_v = 1;

  google::ParseCommandLineFlags(&argc, &argv, true);

  google::InitGoogleLogging(argv[0]);

  auto conf = create_config();

  ld_environment(orig_argv, conf.rules_directory);

  free_args(orig_argc, orig_argv);

  log_config(conf);

  g_core = std::make_shared<core>(new real_core(conf));

  g_core->start();

  g_core.reset();
}
