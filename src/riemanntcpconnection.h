#ifndef RIEMANN_TCP_CONNECTION_H
#define RIEMANN_TCP_CONNECTION_H

#include <tcpconnection.h>
#include <incomingevents.h>

class riemann_tcp_connection : public tcp_connection {
  public:
    bool reading_header;
    uint32_t protobuf_size;
    incoming_events & income_events;

    riemann_tcp_connection(int socket_fd, incoming_events& income_events);
    void callback(int revents);
    void read_cb();
    void write_cb();
    void read_header();
    void read_message();
};

#endif

