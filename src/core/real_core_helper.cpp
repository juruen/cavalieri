#include <glog/logging.h>
#include <core/real_core_helper.h>
#include <util/util.h>
#include <transport/listen_tcp_socket.h>

namespace {

const float k_snapshot_interval = 5;

}

void detach_thread(std::function<void()> fn) {
  std::thread t(fn);
  t.detach();
}

std::shared_ptr<core_interface> make_real_core(const config conf) {

  return std::dynamic_pointer_cast<core_interface>(
            std::make_shared<real_core>(conf));

}

inline void incoming_event(const std::vector<unsigned char> & raw_msg,
                           streams & streams)
{
  riemann::Msg msg;

  if (!msg.ParseFromArray(&raw_msg[0], raw_msg.size())) {
    VLOG(2) << "error parsing protobuf payload";
    return;
  }

  if (msg.has_query()) {
    VLOG(1) << "query messages are not supported yet";
    return;

  }

  streams.process_message(msg);
}

std::unique_ptr<riemann_tcp_pool> init_tcp_server(
    const config & conf,
    main_async_loop_interface & loop,
    streams & streams,
    executor_thread_pool & executor_pool,
    instrumentation & instr
    )
{

  auto income_tcp_event = [&](const std::vector<unsigned char> & raw_msg)
  {
    executor_pool.add_task(
      [=, &streams]() { incoming_event(raw_msg, streams); });
  };

  std::unique_ptr<riemann_tcp_pool> tcp_server(new riemann_tcp_pool(
      conf.riemann_tcp_pool_size,
      income_tcp_event,
      instr
  ));

  auto ptr_server = tcp_server.get();

  loop.add_tcp_listen_fd(create_tcp_listen_socket(conf.events_port),
                         [=](int fd) {ptr_server->add_client(fd); });

  return tcp_server;
}

std::unique_ptr<riemann_udp_pool> init_udp_server(
    const config & conf,
    std::shared_ptr<streams> streams)
{

  auto income_udp_event = [=](const std::vector<unsigned char> raw_msg)
  {
    incoming_event(raw_msg, *streams);
  };

  return std::unique_ptr<riemann_udp_pool>(
      new riemann_udp_pool(conf.events_port, income_udp_event));
}

std::unique_ptr<websocket_pool> init_ws_server(
    const config & conf,
    main_async_loop_interface & loop,
    pub_sub & pubsub,
    real_index & index)
{

  std::unique_ptr<websocket_pool> ws_server(new websocket_pool(
        conf.ws_pool_size, pubsub, index));

  auto ptr_server = ws_server.get();

  loop.add_tcp_listen_fd(create_tcp_listen_socket(conf.ws_port),
                         [=](int fd) {ptr_server->add_client(fd); });

  return ws_server;
}

void snapshot(instrumentation & inst, streams & streams) {

  for (const auto & event : inst.snapshot()) {
    streams.push_event(event);
  }

}


void start_instrumentation(scheduler_interface & sched,
                           instrumentation & instrumentation,
                           streams & streams)
{
  sched.add_periodic_task(
      [&]() { snapshot(instrumentation, streams); },
      k_snapshot_interval
  );
}


