#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include "proto.pb.h"
#include "util.h"
#include "websocket.h"
#include "driver.h"
#include "expression.h"

namespace {
  const uint32_t buffer_size = 1024 * 3;
  const uint32_t max_header_size = 1024 * 3;
  uint32_t listen_backlog = 100;
  const std::string http_end_header("\r\n\r\n");
  const std::string http_end_header_no_r("\n\n");
  const std::string guid("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
}

static bool ends_with(std::string const &str, const std::string &end)
{
  if (str.length() >= end.length()) {
    return (0 == str.compare(str.length() - end.length(), end.length(), end));
  } else {
    return false;
  }
}

static std::vector<std::string> get_lines(const std::string &s) {
  std::vector<std::string> lines;
  std::stringstream ss(s);
  std::string item;

  while (std::getline(ss, item)) {
    while (item.size() > 0 && (item.back() == '\r' || item.back() == '\n')) {
      item.pop_back();
    }
    if (item.size() == 0) {
      continue;
    }
    lines.push_back(item);
  }
  return lines;
}

std::function<bool(const Event&)> filter_query(const std::string uri) {
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

WSConnection::WSConnection(int socket_fd, PubSub& pubsub) :
    sfd(socket_fd), bytes_read(0), bytes_written(0),
    pubsub(pubsub), state(READ_HTTP_HEADER)

{
}

bool WSConnection::write_cb() {
  VLOG(3) << "write_cb()";

  uint32_t old_state = state;
  if (!(state & (WRITE_HTTP_HEADER | WRITE_FRAME))) {
      io.set(ev::READ);
      return true;
  }

  size_t b_to_write = send_buffer.size() - bytes_written;
  VLOG(3) << "Trying to write " <<  b_to_write << " bytes";
  ssize_t nwritten = write(sfd, send_buffer.c_str() + bytes_written, b_to_write);
  VLOG(3) << "Actually written " << nwritten << " bytes";

  if (nwritten < 0) {
    VLOG(3) << "read error: " << strerror(errno);
    return false;
  }

  bytes_written += nwritten;
  if (bytes_written == send_buffer.size()) {
    state = READ_FRAME_HEADER;
    bytes_written = 0;
    send_buffer.clear();
    VLOG(3) << "response sent";
    if (old_state & WRITE_HTTP_HEADER) {
      VLOG(3) << "Header send. Subscribing...";
      auto query_f = filter_query(header_vals.uri);
      pubsub.subscribe(
          "index",
          [=](const evs_list_t& evs)
          {
            VLOG(3) << "Subscribe notification";
            for (auto &e: evs) {
              if (query_f(e)) {
                this->try_send_frame(event_to_json(e));
              }
            }
          },
          reinterpret_cast<uintptr_t>(this)
      );
    }
  }
  return true;
}

void WSConnection::try_send_frame(const std::string& payload) {
  unsigned char header[10];
  header[0] = 129;
  size_t header_length;
  size_t len = payload.size();
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
  state |= WRITE_FRAME;
  set_io();
  VLOG(3) << "try_send_frame() header: " << header_length << " total: " << len;
}

bool WSConnection::try_read_header() {
  char buffer[buffer_size];

  VLOG(3) << "trying to read up to " << buffer_size << " bytes";
  ssize_t nread = recv(sfd, (char*)buffer, buffer_size, 0);
  VLOG(3) << nread << " bytes read in recv()";

  if (nread < 0) {
    VLOG(3) << "read error: " << strerror(errno);
    return true;
  }

  if (nread == 0) {
    VLOG(3) << "peer has disconnected gracefully";
    return false;
  }

  bytes_read += nread;
  if (bytes_read > max_header_size) {
    LOG(ERROR) << "client header too long, closing connection";
    return false;
  }

  header += std::string(buffer, nread);

  if (ends_with(header, http_end_header)
      || ends_with(header, http_end_header_no_r))
  {
    VLOG(3) << "header read: " << header;
    bytes_read = 0;
    return parse_header();
  }

  VLOG(3) << "end of http header not found yet";
  return true;
}

bool WSConnection::parse_header() {
  const std::vector<std::string> lines = get_lines(header);
  if (lines.size() == 0) {
    LOG(ERROR) << "header doesn't contain lines";
    return false;
  }

  /* Parse first line */
  std::vector<std::string> tokens;
  boost::split(tokens, lines[0], boost::is_any_of(" "), boost::token_compress_on);
  header_vals.method = tokens[0];
  header_vals.uri = tokens[1];
  header_vals.version = tokens[2];

  /* Parse reset of header_vals */
  for (uint32_t i = 1; i < lines.size(); i++) {
    size_t pos = lines[i].find(':');
    if (pos == std::string::npos) {
      LOG(ERROR) << "can't parse header: " << lines[i];
      continue;
    }
    const std::string header_name(lines[i].substr(0, pos));
    if ((pos += 2) >= lines[i].size()) {
      LOG(ERROR) << "header content is empty: " << lines[i];
      continue;
    }
    const std::string header_value(lines[i].substr(pos, lines[i].size() - pos));
    header_vals.headers.insert({header_name, header_value});
    VLOG(3) << "header name: " << header_name << " header value: " << header_value;
  }

  for (auto &s : header_vals.headers) {
    VLOG(3) << "header name: " << s.first << " header value: " << s.second;
  }

  return write_response_header();
}

bool WSConnection::write_response_header() {
  auto it = header_vals.headers.find("Sec-WebSocket-Key");
  if (it == header_vals.headers.end()) {
    LOG(ERROR) << "Can't find Sec-WebSocket-Key";
    return false;
  }

  const std::string sha = sha1(it->second + guid);
  const std::string key_response(
      base64Encode(std::vector<unsigned char>(sha.begin(), sha.end())));

  send_buffer =
    "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: "
    + key_response + "\r\n\r\n";

  VLOG(3) << "response: \n" << send_buffer;

  state = WRITE_HTTP_HEADER;
  return true;
}

bool WSConnection::try_read_frame() {
  char buffer[buffer_size];

  VLOG(3) << "trying to read up to " << buffer_size << " bytes";
  ssize_t nread = recv(sfd, (char*)buffer, buffer_size, 0);
  VLOG(3) << nread << " bytes read in recv()";

  if (nread < 0) {
    VLOG(3) << "read error: " << strerror(errno);
    return true;
  }

  if (nread == 0) {
    VLOG(3) << "peer has disconnected gracefully";
    return false;
  }

  bytes_read += nread;
  if (bytes_read > max_header_size) {
    LOG(ERROR) << "client header too long, closing connection";
    return false;
  }

  frames += std::string(buffer, nread);
  if (frames.size() < 2) {
    VLOG(3) << "not enough bytes to figure out frame length";
    return true;
  }

  unsigned char first_byte_len = (unsigned char)frames.c_str()[1] & ~0x80;
  size_t len_bytes = 1;
  if (first_byte_len == 126) {
    len_bytes = 2;
  } else if (first_byte_len == 127) {
    len_bytes = 8;
  }
  VLOG(3) << "first_byte_len: " << (uint32_t)first_byte_len << " len_bytes: " << len_bytes;

  if (frames.size() - 2 < len_bytes) {
    VLOG(3) << "not enough bytes to figure out frame length";
    return true;
  }

  VLOG(3) << "number of length bytes: " << len_bytes;
  uint64_t payload_length = 0;
  switch (len_bytes) {
    case 1:
      payload_length = (uint64_t)((unsigned char)frames.c_str()[1] & ~0x80);
      break;
    case 2:
      uint16_t len16;
      memcpy((void*)&len16, (void*)&frames.c_str()[2], len_bytes);
      payload_length = ntohs(len16);
      break;
    case 8:
      uint64_t len64;
      memcpy((void*)&len64, (void*)&frames.c_str()[2], len_bytes);
      payload_length = be64toh(len64);
      break;
    default:
      LOG(ERROR) << "wrong frame payload_length";
      return false;
  }

  VLOG(3) << "frame payload length: " << payload_length;
  // opcode + len + mask
  size_t header_length = 1 + len_bytes + 4;
  if (len_bytes > 1) {
    header_length += 1;
  }
  size_t total_length = header_length + payload_length;
  VLOG(3) << "frame length: " << total_length;

  if (frames.size() < total_length) {
    VLOG(3) << "not enough bytes for a complete frame";
    return true;
  }

  std::string decoded;
  for (size_t i = header_length, j = 0; i < total_length; i++, j++) {
    decoded += (frames.c_str()[i] ^ (frames.c_str()[(header_length - 4) + (j % 4)])) ;
  }

  VLOG(3) << "frame decoded: " << decoded;

  frames = frames.substr(total_length, frames.size());
  bytes_read -= total_length;

  VLOG(3) << "removing frame from frames and setting bytes_read to " << bytes_read;
  VLOG(3) << "frames size: " << frames.size();

  return true;
}

bool WSConnection::read_cb() {
  if (state == READ_HTTP_HEADER) {
    return try_read_header();
  } else if (state == READ_FRAME_HEADER){
    return try_read_frame();
  }
  return true;
}

void WSConnection::set_io() {
  bool write_response = (state & (WRITE_HTTP_HEADER | WRITE_FRAME));
  VLOG(3) << "setting io to write_response " << write_response;
  if (write_response) {
    io.set(ev::READ | ev::WRITE);
  } else {
    io.set(ev::READ);
  }
}

WSConnection::~WSConnection() {
  io.stop();
  close(sfd);
  pubsub.unsubscribe("index", reinterpret_cast<uintptr_t>(this));
}


void Websocket::io_accept(ev::io &watcher, int revents) {
  if (EV_ERROR & revents) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  int client_sd = accept(watcher.fd, (struct sockaddr *)&client_addr, &client_len);

  if (client_sd < 0) {
    VLOG(3) << "accept error: " << strerror(errno);
    return;
  }

  WSConnection *conn =  new WSConnection(client_sd, pubsub);
  conn_map.insert(std::pair<int, WSConnection*>(client_sd, conn));
  conn->io.set<Websocket, &Websocket::callback>(this);
  conn->io.start(client_sd, ev::READ);

  VLOG(3) << "New connection";

}

void Websocket::remove_connection(WSConnection *conn) {
  conn_map.erase(conn->sfd);
  delete conn;
}

void Websocket::callback(ev::io &watcher, int revents) {
  VLOG(3) << "callback revents " << revents;

  WSConnection *conn = conn_map[watcher.fd];

  if (EV_ERROR & revents) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  bool conn_ok = true;
  if (revents & EV_READ)
    conn_ok = conn->read_cb();

  if (conn_ok && (revents & EV_WRITE))
    conn_ok = conn->write_cb();

  if (conn_ok) {
    conn->set_io();
  } else {
    remove_connection(conn);
  }
}

void Websocket::signal_cb(ev::sig &signal, int revents) {
  UNUSED_VAR(revents);
  signal.loop.break_loop();
}

void Websocket::timer_cb(ev::timer &timer, int revents) {
  UNUSED_VAR(timer);
  UNUSED_VAR(revents);
}

Websocket::Websocket(int port, PubSub& pubsub) : s(port), pubsub(pubsub) {
  VLOG(1) << "Websocket() listening on port " << port;

  struct sockaddr_in addr;

  s = socket(PF_INET, SOCK_STREAM, 0);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    LOG(FATAL) << "bind error: " << strerror(errno);
    exit(1);
  }

  fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) | O_NONBLOCK);

  listen(s, listen_backlog);

  io.set<Websocket, &Websocket::io_accept>(this);
  io.start(s, ev::READ);

  sio.set<&Websocket::signal_cb>();
  sio.start(SIGINT);

  tio.set<Websocket, &Websocket::timer_cb>(this);
  //tio.start(0, 5);
}

Websocket::~Websocket() {
  shutdown(s, SHUT_RDWR);
  close(s);
}

