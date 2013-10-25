#include <vector>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <glog/logging.h>
#include <wsutil.h>
#include <util.h>

namespace {
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

bool ws_util::find_header_end(const std::string& header) {
  return (ends_with(header, http_end_header)
          || ends_with(header, http_end_header_no_r));
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
    VLOG(3) << "header name: " << header_name
            << " header val: " << header_value;
  }

  for (auto &s : header_vals.headers) {
    VLOG(3) << "header name: " << s.first << " header value: " << s.second;
  }

  return true;
}

std::pair<std::string,bool> ws_util::make_header_response() {
  auto it = header_vals.headers.find("Sec-WebSocket-Key");

  if (it == header_vals.headers.end()) {
    LOG(ERROR) << "can't find Sec-WebSocket-Key";
    return {"", false};
  }

  const std::string sha = sha1(it->second + guid);
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


