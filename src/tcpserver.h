#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <ev++.h>
#include <map>
#include <functional>
#include <memory>

typedef std::function<void(int)> add_client_fn_t;

class tcp_server {
  public:
    tcp_server(int port, add_client_fn_t);
    virtual ~tcp_server();
    void io_accept(ev::io &watcher, int revents);
    void callback(ev::io &watcher, int revents);
    static void signal_cb(ev::sig &signal, int revents);

  private:
    int socket_fd;
    ev::io io;
    ev::sig sio;
    add_client_fn_t add_client_fn;
}
;

#endif
