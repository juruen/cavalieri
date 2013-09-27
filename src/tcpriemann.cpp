#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ev++.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <list>
#include <glog/logging.h>
#include "proto.pb.h"

namespace {
  size_t buffer_size = 1024 * 16;
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

//
//   A single instance of a non-blocking TcpRiemann handler
//
class TcpRiemannInstance {
  private:
    ev::io           io;
    static int total_clients;
    int              sfd;
    char             buffer[1025* 8];
    size_t           bytes_read;
    size_t           bytes_written;
    uint32_t         protobuf_size;
    enum State { ReadingHeader, ReadingMessage } state;

    // Buffers that are pending write
    std::list<int>     write_queue;

    // Generic callback
    void callback(ev::io &watcher, int revents) {
      if (EV_ERROR & revents) {
        perror("got invalid event");
        return;
      }

      if (revents & EV_READ)
        read_cb(watcher);

      if (revents & EV_WRITE)
        write_cb(watcher);

      if (write_queue.empty()) {
        io.set(ev::READ);
      } else {
        io.set(ev::READ|ev::WRITE);
      }
    }

    // Socket is writable
    void write_cb(ev::io &watcher) {
      if (write_queue.empty()) {
        io.set(ev::READ);
        return;
      }

      VLOG(3) << "Trying to write " << ok_response_size - bytes_written << " bytes";
      ssize_t nwritten = write(watcher.fd, ok_response + bytes_written , ok_response_size - bytes_written);
      VLOG(3) << "Actually written " << nwritten << " bytes";

      if (nwritten < 0) {
        VLOG(3) << "read error: " << strerror(errno);
        return;
      }

      bytes_written += nwritten;
      if (bytes_written == ok_response_size) {
        write_queue.pop_front();
        bytes_written = 0;
        VLOG(3) << "response sent";
      }
    }

    void try_read_header(ev::io &watcher) {
      VLOG(3) << "trying to read up to " << buffer_size - bytes_read << " bytes";
      ssize_t   nread = recv(watcher.fd, (char*)buffer + bytes_read, buffer_size - bytes_read, 0);
      VLOG(3) << nread << " bytes read in header";

      if (nread < 0) {
        VLOG(3) << "read error: " << strerror(errno);
        return;
      }

      if (nread == 0) {
        VLOG(3) << "we should delete the object";
        io.stop();
        return;
      }

      bytes_read += nread;
      if (bytes_read < 4) {
        return;
      }

      uint32_t header;
      memcpy((void *)&header, buffer, 4);
      protobuf_size = ntohl(header);

      VLOG(2) << "header read. Size of protobuf payload " << protobuf_size << " bytes";

      state = ReadingMessage;
      try_read_message(watcher);
    }

    void try_read_message(ev::io &watcher) {
      if (bytes_read < protobuf_size + 4) {
        VLOG(3) << "trying to read from protobuf payload " << protobuf_size - bytes_read + 4 << " bytes";
        ssize_t   nread = recv(watcher.fd, (char*)buffer + bytes_read, protobuf_size - bytes_read + 4, 0);
        VLOG(3) << "actually read " << nread << " bytes";

        if (nread < 0) {
          VLOG(3) << "read error: " << strerror(errno);
          return;
        }

        if (nread == 0) {
          VLOG(3) << "we should delete the object";
          io.stop();
        }

        bytes_read += nread;
        if ((bytes_read - 4) != protobuf_size) {
          return;
        }
      }

      state = ReadingHeader;
      bytes_read = 0;

      Msg message;
      if (!message.ParseFromArray(buffer + 4, protobuf_size)) {
        VLOG(2) << "error parsing protobuf payload";
        return;
      }

      VLOG(2) << "protobuf payload parsed successfully. nevents " << message.events_size();
      events += message.events_size();

      send_response(watcher);
    }

    void send_response(ev::io &watcher) {
      write_queue.push_back(1);
    }

    // Receive message from client socket
    void read_cb(ev::io &watcher) {
      switch (state) {
        case ReadingHeader:
          try_read_header(watcher);
          break;
        case ReadingMessage:
          try_read_message(watcher);
          break;
      }
    }

    // effictivly a close and a destroy
    virtual ~TcpRiemannInstance() {
      // Stop and free watcher if client socket is closing
      io.stop();
      close(sfd);
      VLOG(3) << "Total connected clients " << --total_clients;
    }
  public:
    TcpRiemannInstance(int s) : sfd(s), bytes_read(0), bytes_written(0), state(ReadingHeader) {
      fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);

      total_clients++;

      io.set<TcpRiemannInstance, &TcpRiemannInstance::callback>(this);

      io.start(s, ev::READ);

      VLOG(3) << "New connection";
    }
};

class TcpRiemannServer {
  private:
    ev::io           io;
    ev::sig         sio;
    ev::timer       tio;
    int                 s;

  public:

    void io_accept(ev::io &watcher, int revents) {
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

      TcpRiemannInstance *client = new TcpRiemannInstance(client_sd);
    }

    static void signal_cb(ev::sig &signal, int revents) {
      signal.loop.break_loop();
    }

    void timer_cb(ev::timer &timer, int revents) {
      VLOG(1) << "Number of events/sec: " << events / 5.0f;
      events = 0;
    }

    TcpRiemannServer(int port) {
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

      io.set<TcpRiemannServer, &TcpRiemannServer::io_accept>(this);
      io.start(s, ev::READ);

      sio.set<&TcpRiemannServer::signal_cb>();
      sio.start(SIGINT);

      tio.set<TcpRiemannServer, &TcpRiemannServer::timer_cb>(this);
      tio.start(0, 5);
    }

    virtual ~TcpRiemannServer() {
      shutdown(s, SHUT_RDWR);
      close(s);
    }
};

int TcpRiemannInstance::total_clients = 0;

int main(int argc, char **argv)
{
  int port = 5555;

  if (argc > 1)
    port = atoi(argv[1]);

  google::InitGoogleLogging(argv[0]);

  ev::default_loop  loop;
  TcpRiemannServer tcpriemann(port);

  loop.run(0);

  return 0;
}
