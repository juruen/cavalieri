#include <netinet/in.h>
#include <sys/socket.h>
#include <glog/logging.h>
#include <algorithm>
#include <proto.pb.h>
#include <riemann_tcp_connection.h>

namespace {

static std::vector<char>  generate_msg_ok()
{
  Msg msg_ok;
  msg_ok.set_ok(true);

  uint32_t nsize = htonl(msg_ok.ByteSize());
  std::vector<char> response (sizeof(nsize) + msg_ok.ByteSize());

  memcpy(&response[0], static_cast<void *>(&nsize), sizeof(nsize));

  CHECK(msg_ok.SerializeToArray(&response[sizeof(nsize)], msg_ok.ByteSize()))
    << "error serialazing msg_ok response";

  return response;
}

const std::vector<char> ok_response = generate_msg_ok();

const size_t k_max_read_iterations = 5;

}

riemann_tcp_connection::riemann_tcp_connection(
    tcp_connection & tcp_connection,
    raw_msg_fn_t raw_msg_fn
) :
  tcp_connection_(tcp_connection),
  raw_msg_fn_(raw_msg_fn),
  reading_header_(true),
  protobuf_size_(0),
  read_iterations_(0)
{
  memcpy(static_cast<void*>(&tcp_connection_.w_buffer[0]),
         &ok_response[0], ok_response.size());
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

  if (reading_header_) {
    read_header();
  } else {
    read_message();
  }

}

void riemann_tcp_connection::write_cb() {

  if (tcp_connection_.bytes_to_write == 0) {
    return;
  }

  if (!tcp_connection_.write()) {
    return;
  }

  if (tcp_connection_.bytes_written == ok_response.size()) {
    tcp_connection_.bytes_to_write = 0;
  }

}

void riemann_tcp_connection::read_header() {

  if (read_iterations_++ > k_max_read_iterations) {

    LOG(ERROR) << "we need to yield cpu time to other connections.";
    read_iterations_ = 0;

    return;
  }

  if (!tcp_connection_.read(4 - tcp_connection_.bytes_read) &&
      tcp_connection_.bytes_read == 0)
  {
    return;
  }

  if (tcp_connection_.bytes_read < 4) {
    return;
  }

  uint32_t header;
  memcpy(static_cast<void *>(&header), &tcp_connection_.r_buffer[0], 4);
  protobuf_size_ = ntohl(header);

  if (protobuf_size_ + 4 > tcp_connection_.buffer_size) {
    LOG(ERROR) << "protobuf_size_ too big: " << protobuf_size_;
    tcp_connection_.close_connection = true;
    return;
  }

  reading_header_ = false;
  read_message();
}

void riemann_tcp_connection::read_message() {

  const auto bytes_read = tcp_connection_.bytes_read;
  const auto frame_size = protobuf_size_ + 4;

  if (bytes_read < frame_size) {

    if (!tcp_connection_.read(frame_size - bytes_read)) {
      return;
    }

    if ((tcp_connection_.bytes_read - 4) != protobuf_size_) {
      return;
    }

  }

  /* We have a complete message */

  tcp_connection_.bytes_to_write = ok_response.size();
  tcp_connection_.bytes_written = 0;

  if (tcp_connection_.bytes_to_write > tcp_connection_.buffer_size) {
    VLOG(2) << "write buffer is full: " << tcp_connection_.bytes_to_write;
    tcp_connection_.close_connection = true;
    return;
  }

  std::vector<unsigned char> msg(protobuf_size_);
  memcpy(&msg[0], &tcp_connection_.r_buffer[4], protobuf_size_);

  /* Process message */
  raw_msg_fn_(std::move(msg));

  /* State transtion */
  reading_header_ = true;

  if (tcp_connection_.bytes_read > frame_size) {

    /* We already have  some bits of the next message */
    tcp_connection_.bytes_read -= frame_size;

    std::rotate(begin(tcp_connection_.r_buffer),
                begin(tcp_connection_.r_buffer) + tcp_connection_.bytes_read,
                end(tcp_connection_.r_buffer));

    read_header();

  } else {

    tcp_connection_.bytes_read = 0;

  }

 }
