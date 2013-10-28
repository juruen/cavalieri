#ifndef RIEMANN_TCP_CONNECTION_H
#define RIEMANN_TCP_CONNECTION_H

#include <tcpconnection.h>
#include <streams.h>

class riemann_tcp_connection : public tcp_connection {
  public:
    bool reading_header;
    uint32_t protobuf_size;
    streams& all_streams;

    riemann_tcp_connection(int socket_fd, streams& all_streams);
    void callback(int revents);
    void read_cb();
    void write_cb();
    void read_header();
    void read_message();
};

#endif

