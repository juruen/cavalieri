#ifndef CAVALIERI_TRANSPORT_WS_CONNECTION_H
#define CAVALIERI_TRANSPORT_WS_CONNECTION_H

#include <transport/tcp_connection.h>
#include <transport/ws_util.h>
#include <async/async_loop.h>

class ws_connection {
  public:
    static const uint32_t  k_read_http_header  = 0x1;
    static const uint32_t  k_write_http_header = 0x2;
    static const uint32_t  k_read_frame_header = 0x4;
    static const uint32_t  k_write_frame       = 0x08;

    ws_connection(tcp_connection & tcp_connection);
    bool send_frame(const std::string & payload);
    uint32_t state() const;
    std::string uri() const;
    void callback(async_fd &);

  private:
    void read_cb();
    void write_cb();
    void read_header();
    void read_frame();
    void write_response_header();

  private:
    tcp_connection & tcp_connection_;
    ws_util ws_util_;
    uint32_t state_;
};

#endif
