#include <netinet/in.h>
#include <iostream>
#include <sys/socket.h>
#include <glog/logging.h>
#include <algorithm>
#include <common/event.h>
#include <riemann_tcp_connection.h>

namespace {

std::vector<char>  generate_msg_ok()
{
  riemann::Msg msg_ok;
  msg_ok.set_ok(true);

  uint32_t nsize = htonl(msg_ok.ByteSize());
  std::vector<char> response (sizeof(nsize) + msg_ok.ByteSize());

  memcpy(&response[0], static_cast<void *>(&nsize), sizeof(nsize));

  CHECK(msg_ok.SerializeToArray(&response[sizeof(nsize)], msg_ok.ByteSize()))
    << "error serialazing msg_ok response";

  return response;
}

const std::vector<char> ok_response(generate_msg_ok());

bool add_ok_response(tcp_connection & connection) {
  VLOG(3) << "adding ok response with size: " << ok_response.size();

  return connection.queue_write(&ok_response[0], ok_response.size());
}

size_t msg_size(tcp_connection & connection) {

  uint32_t header;

  auto p = connection.r_buffer.linearize();

  std::copy(p, p + 4, reinterpret_cast<unsigned char*>(&header));

  connection.r_buffer.erase_begin(4);

  return ntohl(header);

}


}

riemann_tcp_connection::riemann_tcp_connection(
    tcp_connection & tcp_connection,
    raw_msg_fn_t raw_msg_fn
) :
  tcp_connection_(tcp_connection),
  raw_msg_fn_(raw_msg_fn),
  reading_header_(true),
  protobuf_size_(0)
{
}

void riemann_tcp_connection::callback(async_fd & async) {

  if (async.ready_read()) {
    read_cb();
  }

  if (tcp_connection_.close_connection) {
    return;
  }

  if (async.ready_write()) {
    write_cb();
  }
}

void riemann_tcp_connection::read_cb() {

  if (!tcp_connection_.read()) {

    return;

  }

  size_t bytes;

  do {

    bytes = tcp_connection_.read_bytes();

    if (reading_header_) {

      read_header();

    } else {

      read_message();

    }

  } while (bytes != tcp_connection_.read_bytes());

}

void riemann_tcp_connection::write_cb() {

  VLOG(3) << "write_cb(): " << tcp_connection_.pending_write();

  if (!tcp_connection_.pending_write()) {
    return;
  }

  tcp_connection_.write();

}

void riemann_tcp_connection::read_header() {

  VLOG(3) << "reading header";

  if (tcp_connection_.read_bytes() < 4) {
    return;
  }

  protobuf_size_ = msg_size(tcp_connection_);

  if (protobuf_size_ + 4 > tcp_connection_.buff_size) {
    LOG(ERROR) << "msg too big: " << protobuf_size_;
    tcp_connection_.close_connection = true;
    return;
  }

  reading_header_ = false;
}

void riemann_tcp_connection::read_message() {

  if (tcp_connection_.read_bytes() < protobuf_size_) {
    return;
  }

  /* We have a complete message */

  if (!add_ok_response(tcp_connection_)) {
    VLOG(3) << "write buffer is full";
    tcp_connection_.close_connection = true;
    return;
  }


  std::vector<unsigned char> msg(protobuf_size_);

  auto p = tcp_connection_.r_buffer.linearize();

  std::copy(p, p + protobuf_size_, msg.begin());

  tcp_connection_.r_buffer.erase_begin(protobuf_size_);

  /* Process message */
  raw_msg_fn_(std::move(msg));

  /* State transtion */
  reading_header_ = true;

 }
