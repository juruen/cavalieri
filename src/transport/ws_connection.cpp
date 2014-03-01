#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <glog/logging.h>
#include <iostream>
#include <proto.pb.h>
#include <util.h>
#include <transport/ws_connection.h>
#include <query/driver.h>
#include <query/expression.h>

namespace {
  const uint32_t max_header_size = 1024 * 2;
}

ws_connection::ws_connection(
    tcp_connection & tcp_connection
) :
  tcp_connection_(tcp_connection),
  state_(k_read_http_header)
{
}

void ws_connection::write_cb() {

  if (!(state_ & (k_write_http_header | k_write_frame))) {
      tcp_connection_.bytes_to_write = 0;
      return;
  }

  if (!tcp_connection_.write()) {
    return;
  }

  if (tcp_connection_.bytes_to_write > 0) {
    return;
  }

  if (state_ & k_write_http_header) {
    state_ = k_write_frame | k_read_frame_header;
  }

  tcp_connection_.bytes_written = 0;
}

bool ws_connection::send_frame(const std::string & payload) {
  auto buffer = ws_util_.create_frame(payload);
  bool buffer_ok = tcp_connection_.copy_to_write_buffer(buffer.c_str(),
                                                        buffer.size());
  if (!buffer_ok) {
    VLOG(3) << "buffer is full";
    return false;
  }

  state_ |= k_write_frame;
  return true;
}

void ws_connection::read_header() {

  size_t to_read = tcp_connection_.buffer_size - tcp_connection_.bytes_read;
  if (!tcp_connection_.read(to_read)) {
    return;
  }

  if (tcp_connection_.bytes_read > max_header_size) {
    LOG(ERROR) << "client header too long, closing connection";
    tcp_connection_.close_connection = true;
    return;
  }

  const std::string header(reinterpret_cast<char*>(&tcp_connection_.r_buffer[0]),
                           tcp_connection_.bytes_read);

  if (!ws_util_.find_header_end(header))
  {
    return;
  }

  tcp_connection_.bytes_read = 0;

  if (ws_util_.parse_header(header)) {
    write_response_header();
  } else {
    tcp_connection_.close_connection = true;
  }
}

void ws_connection::write_response_header() {
  auto response = ws_util_.make_header_response();

  if (response.second) {

    tcp_connection_.bytes_written = 0;
    bool ok = tcp_connection_.copy_to_write_buffer(response.first.c_str(),
                                                   response.first.size());
    if (!ok) {
      LOG(ERROR) << "can't write to buffer";
      tcp_connection_.close_connection = true;
    }

    state_ = k_write_http_header | k_read_frame_header;
  } else {
    tcp_connection_.close_connection = true;
  }
}

void ws_connection::read_frame() {

  size_t to_read = tcp_connection_.buffer_size - tcp_connection_.bytes_read;

  if (!tcp_connection_.read(to_read)) {
    return;
  }

  if (tcp_connection_.bytes_read > max_header_size) {
    LOG(ERROR) << "client header too long, closing connection";
    tcp_connection_.close_connection = true;
    return;
  }

  auto frame_length = ws_util_.decode_frame(tcp_connection_.r_buffer,
                                            tcp_connection_.bytes_read);

  if (frame_length.malformed_header) {
    tcp_connection_.close_connection = true;
    return;
  }

  if (frame_length.pending_bytes) {
    return;
  }

  /* Got frame decoded */
}

void ws_connection::read_cb() {
  if (state_ | k_read_http_header) {
    read_header();
  } else if (state_ | k_read_frame_header){
    read_frame();
  }
}

void ws_connection::callback(async_fd & async) {

  if (async.ready_read()) {
    read_cb();
  }

  if (tcp_connection_.close_connection) {
    VLOG(3) << "close_connection";
    return;
  }

  if (async.ready_write()) {
    write_cb();
  }

}

uint32_t ws_connection::state() const {
  return state_;
}

std::string ws_connection::uri() const {
  return ws_util_.header_vals.uri;
}
