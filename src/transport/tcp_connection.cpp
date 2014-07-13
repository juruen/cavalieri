#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <glog/logging.h>
#include <transport/tcp_connection.h>
#include <os_functions.h>

namespace {

const size_t k_default_buffer_size = 1024 * 1024;

bool read_from_fd(boost::circular_buffer<unsigned char> & buffer,
                  const int & fd)
{

  VLOG(3) << "read_from_fd() free: " << buffer.reserve();

  unsigned char b[buffer.reserve()];

  ssize_t nread = g_os_functions.recv(fd, &b[0], buffer.reserve(), 0);

  VLOG(3) << "read bytes: " << nread;

  if (nread < 0) {

    LOG(ERROR) << "read error: " << strerror(errno);

    return false;

  }

  if (nread == 0) {

    VLOG(3) << "peer has disconnected gracefully";

    return false;

  }

  buffer.insert(buffer.end(), &b[0], &b[0] + nread);

  return true;

}

bool write_to_fd(boost::circular_buffer<unsigned char> & buffer,
                 const int & fd)
{
  VLOG(3) << "write() up to " << buffer.size() << " bytes";

  auto p = buffer.linearize();
  auto n = g_os_functions.write(fd, p, buffer.size());

  if (n < 0) {
    LOG(ERROR) << "write error: " << strerror(errno);
    return false;
  }

  buffer.erase_begin(n);

  return true;

}



}

tcp_connection::tcp_connection(int sfd) :
  buff_size(k_default_buffer_size),
  sfd(sfd),
  close_connection(false),
  r_buffer(k_default_buffer_size),
  w_buffer(k_default_buffer_size)
{
}

tcp_connection::tcp_connection(int sfd, size_t buff_size):
  buff_size(k_default_buffer_size),
  sfd(sfd),
  close_connection(false),
  r_buffer(buff_size),
  w_buffer(buff_size)
{
}



bool tcp_connection::read() {

  if ((r_buffer.reserve()) <= 0) {
    LOG(ERROR) << "buffer is complete";
    return false;
  }

  if (!read_from_fd(r_buffer, sfd)) {
    close_connection = true;
    return false;
  }

  return true;
}

bool tcp_connection::queue_write(const char *src, const size_t length) {

  VLOG(3) << "queue_write with size: " << length;

  if (w_buffer.reserve() < length) {
    LOG(ERROR) << "error write buffer is full";
    return false;
  }

  w_buffer.insert(w_buffer.end(), src, src + length);

  return true;
}

bool tcp_connection::write() {

  if (!write_to_fd(w_buffer, sfd)) {

    close_connection = true;
    return false;

  } else {

    return true;
  }

}

bool tcp_connection::pending_read() const {
  return true;
}

bool tcp_connection::pending_write() const {
  return w_buffer.size() > 0;
}

size_t tcp_connection::read_bytes() const {
  return r_buffer.size();
}
