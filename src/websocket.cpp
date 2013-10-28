#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <glog/logging.h>
#include <iostream>
#include "proto.pb.h"
#include "util.h"
#include "websocket.h"
#include "driver.h"
#include "expression.h"

namespace {
  const uint32_t max_header_size = 1024 * 2;
  uint32_t  READ_HTTP_HEADER = 0x1;
  uint32_t  WRITE_HTTP_HEADER = 0x2;
  uint32_t  READ_FRAME_HEADER = 0x4;
  uint32_t  WRITE_FRAME = 0x08;
}

static std::function<bool(const Event&)> filter_query(const std::string uri) {
  std::string index;
  std::map<std::string, std::string> params;

  query_f_t true_query = [](const Event&) -> bool { return true; };
  if (!parse_uri(uri, index, params)) {
    return true_query;
  }

  QueryContext query_ctx;
  queryparser::Driver driver(query_ctx);
  if (driver.parse_string(params["query"], "query")) {
    query_ctx.expression->print(std::cout);
    return  query_ctx.expression->evaluate();
  } else {
    return true_query;
  }
}

ws_connection::ws_connection(
  int socket_fd,
  class ws_util* ws_util,
  pub_sub& pubsub
) :
  tcp_connection(socket_fd),
  ws_util(ws_util),
  pubsub(pubsub),
  state(READ_HTTP_HEADER)
{
}

void ws_connection::write_cb() {
  VLOG(3) << "write_cb()";

  if (!(state & (WRITE_HTTP_HEADER | WRITE_FRAME))) {
      io.set(ev::READ);
      return;
  }

  if (!write()) {
    return;
  }

  if (bytes_to_write > 0) {
    return;
  }

  VLOG(3) << "write buffer sent out";

  if (state & WRITE_HTTP_HEADER) {
    VLOG(3) << "header response sent. subscribing client.";
    auto query_f = filter_query(ws_util->header_vals.uri);
    pubsub.subscribe(
        "index",
        [=](const evs_list_t& evs)
        {
          VLOG(3) << "Subscribe notification";
          for (auto &e: evs) {
            if (query_f(e)) {
              this->send_frame(event_to_json(e));
            }
          }
        },
        reinterpret_cast<uintptr_t>(this)
        );
  }
  state = READ_FRAME_HEADER;
  bytes_written = 0;
}

void ws_connection::send_frame(const std::string& payload) {
  VLOG(3) << "send_frame() payload: " << payload;
  unsigned char header[10];
  header[0] = 129;
  size_t header_length;
  size_t len = payload.size();
  std::string send_buffer;
  if (payload.size() <= 125) {
    header[1] = (unsigned char)len;
    header_length = 2;
  } else if (len >= 126 && len <= 65535) {
    header[1] = 126;
    uint16_t nlen = htons((uint16_t)len);
    memcpy((void*)&header[2], (void*)&nlen, 2);
    header_length = 4;
  } else {
    header[1] = 127;
    uint64_t nlen = htons((uint16_t)len);
    memcpy((void*)&header[2], (void*)&nlen, 8);
    header_length = 10;
  }
  for (size_t i = 0; i < header_length; i++) {
    send_buffer += header[i];
  }
  send_buffer += payload;
  copy_to_write_buffer(send_buffer.c_str(), send_buffer.size());
  state |= WRITE_FRAME;
  set_io();
  VLOG(3) << "send_frame() header: " << header_length << " total: " << len;
}

void ws_connection::read_header() {
  VLOG(3) << "read_header()";

  if (!read(buffer_size - bytes_read)) {
    return;
  }

  if (bytes_read > max_header_size) {
    LOG(ERROR) << "client header too long, closing connection";
    close_connection = true;
    return;
  }

  const std::string header(r_buffer, bytes_read);

  if (!ws_util->find_header_end(header))
  {
    VLOG(3) << "end of http header not found yet";
    return;
  }

  VLOG(3) << "header read: " << header;
  bytes_read = 0;

  if (ws_util->parse_header(header)) {
    write_response_header();
  } else {
    //Failed to parse header
    close_connection = true;
  }
}

void ws_connection::write_response_header() {
  auto response = ws_util->make_header_response();
  if (response.second) {
    VLOG(3) << "response: \n" << response.first;
    bytes_written = 0;
    bool ok = copy_to_write_buffer(response.first.c_str(),
                                   response.first.size());
    if (!ok) {
      LOG(ERROR) << "can't write to buffer";
      close_connection = true;
    }
    state = WRITE_HTTP_HEADER;
  } else {
    VLOG(3) << "failed to create response header";
    close_connection = true;
  }
}

void ws_connection::read_frame() {
  VLOG(3) << "read_frame()";

  if (!read(buffer_size - bytes_read)) {
    return;
  }

  if (bytes_read > max_header_size) {
    LOG(ERROR) << "client header too long, closing connection";
    close_connection = true;
    return;
  }

  if (bytes_read < 2) {
    VLOG(3) << "not enough bytes to figure out frame length";
    return;
  }

  unsigned char first_byte_len = (unsigned char)r_buffer[1] & ~0x80;
  size_t len_bytes = 1;
  if (first_byte_len == 126) {
    len_bytes = 2;
  } else if (first_byte_len == 127) {
    len_bytes = 8;
  }
  VLOG(3) << "first_byte_len: " << (uint32_t)first_byte_len
          << " len_bytes: " << len_bytes;

  if (bytes_read - 2 < len_bytes) {
    VLOG(3) << "not enough bytes to figure out frame length";
    return;
  }

  VLOG(3) << "number of length bytes: " << len_bytes;
  uint64_t payload_length = 0;
  switch (len_bytes) {
    case 1:
      payload_length = (uint64_t)((unsigned char)r_buffer[1] & ~0x80);
      break;
    case 2:
      uint16_t len16;
      memcpy((void*)&len16, (void*)&r_buffer[2], len_bytes);
      payload_length = ntohs(len16);
      break;
    case 8:
      uint64_t len64;
      memcpy((void*)&len64, (void*)&r_buffer[2], len_bytes);
      payload_length = be64toh(len64);
      break;
    default:
      LOG(ERROR) << "wrong frame payload_length";
      close_connection = true;
      return;
  }

  VLOG(3) << "frame payload length: " << payload_length;

  // opcode + len + mask
  size_t header_length = 1 + len_bytes + 4;
  if (len_bytes > 1) {
    header_length += 1;
  }
  size_t total_length = header_length + payload_length;
  VLOG(3) << "frame length: " << total_length;

  if (bytes_read < total_length) {
    VLOG(3) << "not enough bytes for a complete frame";
    return;
  }

  std::string decoded;
  for (size_t i = header_length, j = 0; i < total_length; i++, j++) {
    decoded += (r_buffer[i] ^ (r_buffer[(header_length - 4) + (j % 4)])) ;
  }

  VLOG(3) << "frame decoded: " << decoded;

  //XXX
  /*
  frames = frames.substr(total_length, frames.size());
  bytes_read -= total_length;

  VLOG(3) << "removing frame from frames and setting bytes_read to " << bytes_read;
  VLOG(3) << "frames size: " << frames.size();
  */
  return;
}

void ws_connection::read_cb() {
  if (state == READ_HTTP_HEADER) {
    read_header();
  } else if (state == READ_FRAME_HEADER){
    read_frame();
  }
}

void ws_connection::callback(int revents) {
  VLOG(3) << "callback() ";

  if (revents & EV_READ) {
    VLOG(4) << "entering read_cb()";
    read_cb();
  }

  if (close_connection) {
    VLOG(3) << "close_connection";
    return;
  }

  if (revents & EV_WRITE) {
    VLOG(3) << "entering write_cb()";
    write_cb();
  }
}

ws_connection::~ws_connection() {
  io.stop();
  close(sfd);
  pubsub.unsubscribe("index", reinterpret_cast<uintptr_t>(this));
  if (ws_util) {
    delete ws_util;
  }
}
