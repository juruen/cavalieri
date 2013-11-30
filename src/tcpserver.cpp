#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <glog/logging.h>
#include <proto.pb.h>
#include <tcpserver.h>
#include <tcpconnection.h>
#include <util.h>

namespace {
  const uint32_t listen_backlog = 100;
}

void tcp_server::io_accept(ev::io &watcher, int revents) {
  if (EV_ERROR & revents) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);
  int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr,
                         &client_len);

  if (client_sd < 0) {
    VLOG(3) << "accept error: " << strerror(errno);
    return;
  }

  add_client_fn(client_sd);

  VLOG(3) << "io_accept() new connection";
}

void tcp_server::signal_cb(ev::sig &signal, int revents) {
  VLOG(3) << "signal reciived";
  signal.loop.break_loop();
  UNUSED_VAR(revents);
}

tcp_server::tcp_server( int port, add_client_fn_t add_client_fn)
    :
  add_client_fn(add_client_fn)
{
  VLOG(1) << "tcp_server() port: " << port;

  struct sockaddr_in addr;
  socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    LOG(FATAL) << "bind error: " << strerror(errno);
    exit(1);
  }

  fcntl(socket_fd, F_SETFL, fcntl(socket_fd, F_GETFL, 0) | O_NONBLOCK);
  listen(socket_fd, listen_backlog);
  io.set<tcp_server, &tcp_server::io_accept>(this);
  io.start(socket_fd, ev::READ);
  sio.set<&tcp_server::signal_cb>();
  sio.start(SIGINT);
}

tcp_server::~tcp_server() {
  shutdown(socket_fd, SHUT_RDWR);
  close(socket_fd);
}

