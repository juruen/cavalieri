#include <vector>
#include <sstream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>
#include <transport/ws_util.h>
#include <util.h>

namespace {

const std::string k_http_header_end("\r\n\r\n");
const std::string k_http_header_end_no_r("\n\n");
const std::string k_guid("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");

bool ends_with(std::string const &str, const std::string &end)
{

  if (str.length() >= end.length()) {
    return (0 == str.compare(str.length() - end.length(), end.length(), end));
  }

  return false;

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

}

bool ws_util::find_header_end(const std::string& header) {

  return (ends_with(header, k_http_header_end)
          || ends_with(header, k_http_header_end_no_r));

}

bool ws_util::parse_header(const std::string& header) {

  const std::vector<std::string> lines = get_lines(header);

  if (lines.size() == 0) {
    LOG(ERROR) << "header doesn't contain lines";
    return false;
  }

  /* Parse first line */
  std::vector<std::string> tokens;
  boost::split(tokens, lines[0], boost::is_any_of(" "),
               boost::token_compress_on);

  header_vals.method = tokens[0];
  header_vals.uri = tokens[1];
  header_vals.version = tokens[2];

  /* Parse rest of header_vals */
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
  }

  return true;
}

std::pair<std::string,bool> ws_util::make_header_response() {

  auto it = header_vals.headers.find("Sec-WebSocket-Key");

  if (it == header_vals.headers.end()) {
    LOG(ERROR) << "can't find Sec-WebSocket-Key";
    return {"", false};
  }

  const std::string sha = sha1(it->second + k_guid);
  const std::string key_response(
      base64Encode(std::vector<unsigned char>(sha.begin(), sha.end())));

  std::string response =
    "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: "
    + key_response + "\r\n\r\n";

  return {response, true};
}


std::string ws_util::create_frame(const std::string payload) {

  std::vector<unsigned char> header(10);

  header[0] = 129;

  size_t header_length;

  size_t len = payload.size();

  if (payload.size() <= 125) {

    header[1] = static_cast<unsigned char>(len);
    header_length = 2;

  } else if (len >= 126 && len <= 65535) {

    header[1] = 126;
    uint16_t nlen = htons(static_cast<uint16_t>(len));

    memcpy(static_cast<void*>(&header[2]), static_cast<void*>(&nlen), 2);
    header_length = 4;

  } else {

    header[1] = 127;
    uint64_t nlen = htons((uint16_t)len);

    memcpy(static_cast<void*>(&header[2]), static_cast<void*>(&nlen), 8);
    header_length = 10;

  }

  std::string buffer;
  std::copy(header.begin(), header.begin() + header_length,
            back_inserter(buffer));
  std::copy(payload.begin(), payload.end(), back_inserter(buffer));

  return buffer;
}

ws_util::frame_length_t ws_util::decode_frame(
  const std::vector<unsigned char> buffer, size_t buffer_len)
{

  if (buffer_len < 2) {
    return {false, true, 0, 0, {}}; // Pending bytes
  }

  auto first_byte_len = buffer[1] & ~0x80;

  size_t len_bytes;

  switch (first_byte_len) {

    case 126:

      len_bytes = 2;
      break;

    case 127:

        len_bytes = 8;
        break;

    default:

        len_bytes = 1;
  };

  if (buffer_len - 2 < len_bytes) {
    return {false, true, 0, 0, {}}; // Pending bytes
  }

  uint64_t payload_length;

  switch (len_bytes) {

    case 1:

      payload_length = static_cast<uint64_t>(buffer[1] & ~0x80);

      break;

    case 2:

      uint16_t len16;

      memcpy(static_cast<void*>(&len16), static_cast<const void*>(&buffer[2]),
            len_bytes);

      payload_length = ntohs(len16);

      break;

    case 8:

      uint64_t len64;

      memcpy(static_cast<void*>(&len64), static_cast<const void*>(&buffer[2]),
             len_bytes);

      payload_length = be64toh(len64);

      break;

    default:

      LOG(ERROR) << "wrong frame payload_length";
      return {true, false, 0, 0, {}}; // Error. Hint to close connection.
  }

  // opcode + len + mask
  size_t header_length = 1 + len_bytes + 4;
  if (len_bytes > 1) {
    header_length += 1;
  }

  auto total_length = header_length + payload_length;
  if (buffer_len < total_length) {
    VLOG(3) << "not enough bytes for a complete frame";
    return {false, true, header_length, payload_length, {}};
  }

  std::string decoded;
  for (size_t i = header_length, j = 0; i < total_length; i++, j++) {
    decoded += (buffer[i] ^ (buffer[(header_length - 4) + (j % 4)])) ;
  }

  return {false, false, header_length, payload_length, std::move(decoded)};
}
