#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <ev++.h>
#include <map>

struct TCPConnection {
    int sfd;
    size_t bytes_read;
    size_t bytes_written;
    bool reading_header;
    bool write_response;
    uint32_t protobuf_size;
    char buffer[1024*8];
    ev::io io;

    TCPConnection(int socket_fd);
    virtual ~TCPConnection();
    void set_io();
    bool read_cb();
    bool write_cb();
    bool try_read_header();
    bool try_read_message();
};

class TCPServer {
  private:
    ev::io io;
    ev::sig sio;
    ev::timer tio;
    int s;
    std::map<const int, TCPConnection*> conn_map;

  public:
    TCPServer(int port);
    void io_accept(ev::io &watcher, int revents);
    void remove_connection(TCPConnection* conn);
    void callback(ev::io &watcher, int revents);
    static void signal_cb(ev::sig &signal, int revents);
    void timer_cb(ev::timer &timer, int revents);
    virtual ~TCPServer();
};

#endif
