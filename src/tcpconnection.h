#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <ev++.h>

class tcp_connection {
  public:
    int sfd;
    size_t bytes_to_read;
    size_t bytes_to_write;
    size_t bytes_read;
    size_t bytes_written;
    bool close_connection;
    static const uint32_t buffer_size = 1024 * 64;
    char r_buffer[buffer_size];
    char w_buffer[buffer_size];
    ev::io io;

    tcp_connection(int socket_fd);
    virtual ~tcp_connection();

    bool read(const uint32_t& to_read);
    bool copy_to_write_buffer(const char *src, uint32_t length);
    bool write();
    virtual void callback(int revents) = 0;
    void set_io();
};

#endif
