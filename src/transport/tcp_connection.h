#ifndef TRANSPORT_TCP_CONNECTION_H
#define TRANSPORT_TCP_CONNECTION_H

#include <vector>

class tcp_connection {
  public:
    tcp_connection(int socket_fd);
    tcp_connection(int socket_fd, size_t buff_size);

    bool read(const uint32_t & to_read);
    bool copy_to_write_buffer(const char *src, uint32_t length);
    bool write();
    bool pending_write() const;
    bool pending_read() const;

    int sfd;
    size_t bytes_to_read;
    size_t bytes_to_write;
    size_t bytes_read;
    size_t bytes_written;
    bool close_connection;
    size_t buffer_size;
    std::vector<unsigned char> r_buffer;
    std::vector<unsigned char> w_buffer;

};

#endif
