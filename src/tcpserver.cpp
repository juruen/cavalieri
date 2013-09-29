#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <glog/logging.h>
#include "proto.pb.h"
#include "tcpserver.h"

namespace {
  const uint32_t buffer_size = 1024 * 16;
  uint32_t events = 0;
  char ok_response[1024];
  uint32_t ok_response_size;
  uint32_t listen_backlog = 100;
}

static void generate_msg_ok()
{
  Msg msg_ok;
  msg_ok.set_ok(true);

  uint32_t nsize = htonl(msg_ok.ByteSize());
  memcpy(ok_response, (void *)&nsize, sizeof(nsize));
  if (!msg_ok.SerializeToArray(ok_response + sizeof(nsize), msg_ok.ByteSize())) {
    VLOG(1) << "Error serializing response\n";
    exit(1);
  }

  ok_response_size = sizeof(nsize) + msg_ok.ByteSize();
}

TCPConnection::TCPConnection(int socket_fd) :
    sfd(socket_fd), bytes_read(0), bytes_written(0),
    reading_header(true), write_response(false)
{
}

bool TCPConnection::write_cb() {
  VLOG(3) << "write_cb()";

  if (!write_response) {
    io.set(ev::READ);
    return true;
  }

  VLOG(3) << "Trying to write " << ok_response_size - bytes_written << " bytes";
  ssize_t nwritten = write(sfd, ok_response + bytes_written ,ok_response_size - bytes_written);
  VLOG(3) << "Actually written " << nwritten << " bytes";

  if (nwritten < 0) {
    VLOG(3) << "read error: " << strerror(errno);
    return false;
  }

  bytes_written += nwritten;
  if (bytes_written == ok_response_size) {
    write_response = false;
    bytes_written = 0;
    VLOG(3) << "response sent";
  }
  return true;
}

bool TCPConnection::try_read_header() {
  VLOG(3) << "trying to read up to " << buffer_size - bytes_read << " bytes";
  ssize_t nread = recv(sfd, (char*)buffer + bytes_read, buffer_size - bytes_read, 0);
  VLOG(3) << nread << " bytes read in header";

  if (nread < 0) {
    VLOG(3) << "read error: " << strerror(errno);
    return true;
  }

  if (nread == 0) {
    VLOG(3) << "peer has disconnected gracefully";
    return false;
  }

  bytes_read += nread;
  if (bytes_read < 4) {
    return true;
  }

  uint32_t header;
  memcpy((void *)&header, buffer, 4);
  protobuf_size = ntohl(header);

  VLOG(2) << "header read. Size of protobuf payload " << protobuf_size << " bytes";

  reading_header = false;

  return try_read_message();
}

bool TCPConnection::try_read_message() {

  if (bytes_read < protobuf_size + 4) {
    VLOG(3) << "trying to read from protobuf payload " << protobuf_size - bytes_read + 4 << " bytes";
    ssize_t  nread = recv(sfd, (char*)buffer + bytes_read, protobuf_size - bytes_read + 4, 0);
    VLOG(3) << "actually read " << nread << " bytes";

    if (nread < 0) {
      VLOG(3) << "read error: " << strerror(errno);
      return true;
    }

    if (nread == 0) {
      VLOG(3) << "peer has disconnected gracefully";
      return false;
    }

    bytes_read += nread;
    if ((bytes_read - 4) != protobuf_size) {
      return true;
    }
  }

  reading_header = true;
  bytes_read = 0;

  Msg message;
  if (!message.ParseFromArray(buffer + 4, protobuf_size)) {
    VLOG(2) << "error parsing protobuf payload";
    return true;
  }

  VLOG(2) << "protobuf payload parsed successfully. nevents " << message.events_size();
  events += message.events_size();

  write_response = true;

  return true;
}

bool TCPConnection::read_cb() {
  if (reading_header) {
    return try_read_header();
  } else {
    return try_read_message();
  }
}

void TCPConnection::set_io() {
  io.set(ev::READ | write_response ?  ev::WRITE : 0);
}

void TCPConnection::stop() {
  io.stop();
  close(sfd);
}


void TCPServer::io_accept(ev::io &watcher, int revents) {
  if (EV_ERROR & revents) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    VLOG(3) << "accept error: " << strerror(errno);
    return;
  }

  TCPConnection *conn =  new TCPConnection(client_sd);
  conn_map.insert(std::pair<int, TCPConnection*>(client_sd, conn));
  conn->io.set<TCPServer, &TCPServer::callback>(this);
  conn->io.start(client_sd, ev::READ);

  VLOG(3) << "New connection";

}

void TCPServer::remove_connection(TCPConnection *conn) {
  conn->stop();
  conn_map.erase(conn->sfd);
  delete conn;
}

void TCPServer::callback(ev::io &watcher, int revents) {
  VLOG(3) << "callback revents " << revents;

  TCPConnection *conn = conn_map[watcher.fd];

  if (EV_ERROR & revents) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  bool conn_ok;
  if (revents & EV_READ)
    conn_ok = conn->read_cb();

  if (conn_ok && (revents & EV_WRITE))
    conn_ok = conn->write_cb();

  if (conn_ok) {
    conn->set_io();
  } else {
    remove_connection(conn);
  }
}

void TCPServer::signal_cb(ev::sig &signal, int revents) {
  signal.loop.break_loop();
}

void TCPServer::timer_cb(ev::timer &timer, int revents) {
  VLOG(1) << "Number of events/sec: " << events / 5.0f;
  events = 0;
}

TCPServer::TCPServer(int port) {
  VLOG(1) << "Listening on port" << port;

  generate_msg_ok();

  struct sockaddr_in addr;

  s = socket(PF_INET, SOCK_STREAM, 0);

  addr.sin_family = AF_INET;
  addr.sin_port     = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    LOG(FATAL) << "bind error: " << strerror(errno);
    exit(1);
  }

  fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);

  listen(s, listen_backlog);

  io.set<TCPServer, &TCPServer::io_accept>(this);
  io.start(s, ev::READ);

  sio.set<&TCPServer::signal_cb>();
  sio.start(SIGINT);

  tio.set<TCPServer, &TCPServer::timer_cb>(this);
  tio.start(0, 5);
}

TCPServer::~TCPServer() {
  shutdown(s, SHUT_RDWR);
  close(s);
}

