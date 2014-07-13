#ifndef RIEMANN_TCP_CONNECTION_H
#define RIEMANN_TCP_CONNECTION_H

#include <functional>
#include <vector>
#include <cstddef>
#include <transport/tcp_connection.h>
#include <async/async_loop.h>

typedef std::function<void(std::vector<unsigned char>)> raw_msg_fn_t;

class riemann_tcp_connection {
  public:
    riemann_tcp_connection(
        tcp_connection & tcp_connection_,
        raw_msg_fn_t raw_msg_fn
    );
    void callback(async_fd &);

  private:
    void read_cb();
    void write_cb();
    void read_header(bool);
    void read_message();

  private:
    tcp_connection & tcp_connection_;
    raw_msg_fn_t raw_msg_fn_;
    bool reading_header_;
    size_t protobuf_size_;
};

#endif

