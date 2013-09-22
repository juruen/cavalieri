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
#include "proto.pb.h"

namespace {
  char response[] = "HTTP/1.0 200 OK\nServer: SimpleHTTP/0.6 Python/2.7.4\nDate: Sun, 22 Sep 2013 09:49:26 GMT\nContent-type: text/html; charset=UTF-8\nContent-Length: 0\n\n";
  size_t buffer_size = 1024 * 8;
  Msg msg_ok;
}
//
//   Buffer class - allow for output buffering such that it can be written out 
//                                 into async pieces
//
struct Buffer {
  char       *data;
  ssize_t len;
  ssize_t pos;

  Buffer(const char *bytes, ssize_t nbytes) {
    pos = 0;
    len = nbytes;
    data = new char[nbytes];
    memcpy(data, bytes, nbytes);
  }

  virtual ~Buffer() {
    delete [] data;
  }

  char *dpos() {
    return data + pos;
  }

  ssize_t nbytes() {
    return len - pos;
  }
};


//
//   A single instance of a non-blocking TcpRiemann handler
//
class TcpRiemannInstance {
  private:
    ev::io           io;
    static int total_clients;
    int              sfd;

    // Buffers that are pending write
    std::list<Buffer*>     write_queue;

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

      Buffer* buffer = write_queue.front();

      ssize_t written = write(watcher.fd, buffer->dpos(), buffer->nbytes());
      if (written < 0) {
        perror("read error");
        return;
      }

      buffer->pos += written;
      if (buffer->nbytes() == 0) {
        write_queue.pop_front();
        delete buffer;
      }
    }

    // Receive message from client socket
    void read_cb(ev::io &watcher) {
      uint32_t header;

      ssize_t   nread = recv(watcher.fd, &header, sizeof(header), 0);

      if (nread < 0) {
        perror("read error");
        return;
      }

      if (nread == 0) {
        printf("deleting object\n");
        delete this;
      }

      if (nread != sizeof(header)) {
        printf("Error reading header\n");
        return;
      }

      uint32_t protobuf_size = ntohl(header);
      printf("Size of protbuf %i\n",  protobuf_size);
      if (protobuf_size > buffer_size) {
        printf("Error header size is greater than buffer size\n");
        return;
      }

      // Try to read all events
      //
      char buffer[buffer_size];
      nread = recv(watcher.fd, buffer, protobuf_size, 0);

      if (nread != protobuf_size) {
        printf("Read bytes and protobuf_size don't match\n");
        return;
      }

      Msg message;
      if (!message.ParseFromArray(buffer, protobuf_size)) {
        printf("Error parsing message\n");
        return;
      }

      printf("Message parsed successfully\n");

      uint32_t nsize = htonl(msg_ok.ByteSize());
      memcpy(buffer, (void *)&nsize, sizeof(nsize));
      if (!msg_ok.SerializeToArray(buffer + sizeof(nsize), msg_ok.ByteSize())) {
        printf("Error serializing response\n");
        return;
      }
      printf("Message serialized successfully. Size %i\n", msg_ok.ByteSize());
      write_queue.push_back(new Buffer(buffer, sizeof(nsize) + msg_ok.ByteSize()));
      /*
      if (nread == 0) {
        // Gack - we're deleting ourself inside of ourself!
        delete this;
      } else {
        // Send message bach to the client
        //write_queue.push_back(new Buffer(buffer, nread));
        write_queue.push_back(new Buffer(response, sizeof(response)));
      }*/
    }

    // effictivly a close and a destroy
    virtual ~TcpRiemannInstance() {
      // Stop and free watcher if client socket is closing
      io.stop();

      close(sfd);

      //printf("%d client(s) connected.\n", --total_clients);
    }

  public:
    TcpRiemannInstance(int s) : sfd(s) {
      fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);

      //printf("Got connection\n");
      total_clients++;

      io.set<TcpRiemannInstance, &TcpRiemannInstance::callback>(this);

      io.start(s, ev::READ);
    }
};

class TcpRiemannServer {
  private:
    ev::io           io;
    ev::sig         sio;
    int                 s;

  public:

    void io_accept(ev::io &watcher, int revents) {
      if (EV_ERROR & revents) {
        perror("got invalid event");
        return;
      }

      struct sockaddr_in client_addr;
      socklen_t client_len = sizeof(client_addr);

      int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);

      if (client_sd < 0) {
        perror("accept error");
        return;
      }

      TcpRiemannInstance *client = new TcpRiemannInstance(client_sd);
    }

    static void signal_cb(ev::sig &signal, int revents) {
      signal.loop.break_loop();
    }

    TcpRiemannServer(int port) {
      printf("Listening on port %d\n", port);

      struct sockaddr_in addr;

      s = socket(PF_INET, SOCK_STREAM, 0);

      addr.sin_family = AF_INET;
      addr.sin_port     = htons(port);
      addr.sin_addr.s_addr = INADDR_ANY;

      if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("bind");
      }

      fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);

      listen(s, 5);

      io.set<TcpRiemannServer, &TcpRiemannServer::io_accept>(this);
      io.start(s, ev::READ);

      sio.set<&TcpRiemannServer::signal_cb>();
      sio.start(SIGINT);
    }

    virtual ~TcpRiemannServer() {
      shutdown(s, SHUT_RDWR);
      close(s);
    }
};

int TcpRiemannInstance::total_clients = 0;

int main(int argc, char **argv)
{
  int         port = 5555;

  if (argc > 1)
    port = atoi(argv[1]);

  msg_ok.set_ok(true);
  ev::default_loop       loop;
  TcpRiemannServer                   echo(port);

  loop.run(0);

  return 0;
}
