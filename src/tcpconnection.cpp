#include <sys/types.h>
#include <sys/socket.h>
#include <glog/logging.h>
#include <tcpconnection.h>
#include <os_functions.h>

namespace {
  const size_t k_default_buffer_size = 1024 * 128;
}

tcp_connection::tcp_connection(int sfd) :
  sfd(sfd),
  bytes_to_read(0),
  bytes_to_write(0),
  bytes_read(0),
  bytes_written(0),
  close_connection(false),
  buffer_size(k_default_buffer_size),
  r_buffer(k_default_buffer_size),
  w_buffer(k_default_buffer_size)
{
}

tcp_connection::tcp_connection(int sfd, size_t buff_size):
  sfd(sfd),
  bytes_to_read(0),
  bytes_to_write(0),
  bytes_read(0),
  bytes_written(0),
  close_connection(false),
  buffer_size(buff_size),
  r_buffer(buff_size),
  w_buffer(buff_size)
{
}



bool tcp_connection::read(const uint32_t & to_read) {

  ssize_t nread = g_os_functions.recv(sfd, &r_buffer[0] + bytes_read,
                                      to_read, 0);

  if (nread < 0) {
    VLOG(3) << "read error: " << strerror(errno);
    return true;
  }

  if (nread == 0) {
    VLOG(3) << "peer has disconnected gracefully";
    close_connection = true;
    return false;
  }

  bytes_read += nread;

  return true;
}

bool tcp_connection::copy_to_write_buffer(const char *src, uint32_t length) {

  if ((bytes_to_write + length) > buffer_size) {
    LOG(ERROR) << "error write buffer is full";
    return false;
  }

  memcpy(&w_buffer[0] + bytes_to_write, static_cast<const void*>(src), length);

  bytes_to_write += length;

  return true;
}

bool tcp_connection::write() {
  VLOG(3) << "write() up to " << bytes_to_write << " bytes";

  ssize_t nwritten = g_os_functions.write(sfd, &w_buffer[0] + bytes_written,
                                          bytes_to_write);

  if (nwritten < 0) {
    VLOG(3) << "write error: " << strerror(errno);
    return true;
  }

  bytes_written += nwritten;
  bytes_to_write -= nwritten;

  return true;
}

bool tcp_connection::pending_read() const {
  return true;
}

bool tcp_connection::pending_write() const {
  return (bytes_to_write > 0);
}
