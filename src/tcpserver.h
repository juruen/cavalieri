#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <ev++.h>
#include <map>
#include <functional>
#include <memory>

class tcp_connection;
typedef std::function<std::shared_ptr<tcp_connection>(int)>
        create_tcp_connection_f_t;

class tcp_server {
  private:
    create_tcp_connection_f_t create_tcp_connection_f;
    int socket_fd;
    ev::io io;
    ev::sig sio;
    std::map<const int, std::shared_ptr<tcp_connection>> conn_map;

  public:
    tcp_server(int port, create_tcp_connection_f_t create_tcp_connection_f);
    virtual ~tcp_server();
    void io_accept(ev::io &watcher, int revents);
    void remove_connection(std::shared_ptr<tcp_connection> conn);
    void callback(ev::io &watcher, int revents);
    static void signal_cb(ev::sig &signal, int revents);
};

#endif
