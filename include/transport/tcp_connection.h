#ifndef CAVALIERI_TRANSPORT_TCP_CONNECTION_H
#define CAVALIERI_TRANSPORT_TCP_CONNECTION_H

#include <vector>
#include <boost/circular_buffer.hpp>

class tcp_connection {
  public:
    tcp_connection(int socket_fd);
    tcp_connection(int socket_fd, size_t buff_size);

    bool read();
    bool queue_write(const char *src, size_t length);
    bool write();
    bool pending_write() const;
    bool pending_read() const;
    size_t read_bytes() const;

    const size_t buff_size;
    int sfd;
    bool close_connection;
    boost::circular_buffer<unsigned char> r_buffer;
    boost::circular_buffer<unsigned char> w_buffer;

};


#endif
