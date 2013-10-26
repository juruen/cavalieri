#include <sys/types.h>
#include <sys/socket.h>
#include <glog/logging.h>
#include <tcpconnection.h>

tcp_connection::tcp_connection(int sfd) :
  sfd(sfd),
  bytes_to_read(0),
  bytes_to_write(0),
  bytes_read(0),
  bytes_written(0),
  close_connection(false)
{
}


bool tcp_connection::read(const uint32_t& to_read) {
  VLOG(3) << "read() up to  " << to_read << " bytes";

  ssize_t nread = recv(sfd, (char*)r_buffer + bytes_read, to_read, 0);

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
  VLOG(3) << "just read: " << nread << " bytes. total so far: " << bytes_read;

  return true;
}

bool tcp_connection::copy_to_write_buffer(const char *src, uint32_t length) {
  VLOG(3) << "copy_to_write_buffer() from: " << bytes_to_write
          << " to " << bytes_written + length;
  if ((bytes_to_write + length) > buffer_size) {
    LOG(ERROR) << "error write buffer is full";
    return false;
  }
  memcpy((void*)(w_buffer + bytes_to_write), (void*)src, length);
  bytes_to_write += length;
  return true;
}

bool tcp_connection::write() {
  VLOG(3) << "write() up to " << bytes_to_write << " bytes";

  ssize_t nwritten = ::write(sfd, w_buffer + bytes_written, bytes_to_write);

  if (nwritten < 0) {
    VLOG(3) << "write error: " << strerror(errno);
    return true;
  }

  bytes_written += nwritten;
  bytes_to_write -= nwritten;

  return true;
}

void tcp_connection::set_io() {
  if (bytes_to_write > 0) {
    VLOG(3) << "set_io() set to RW";
    io.set(ev::READ | ev::WRITE);
  } else {
    VLOG(3) << "set_io() set to R";
    io.set(ev::READ);
  }
}

tcp_connection::~tcp_connection() {
  VLOG(3) << "~tcp_connection()";
  io.stop();
  close(sfd);
}
