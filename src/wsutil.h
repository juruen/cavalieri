#ifndef WS_UTIL_H
#define WS_UTIL_H

#include <string>
#include <unordered_map>

class ws_util {
  public:
    struct header_values {
      std::string method;
      std::string uri;
      std::string version;
      std::unordered_map<std::string, const std::string> headers;
    } header_vals;

    bool find_header_end(const std::string& header);
    bool parse_header(const std::string& header);
    std::pair<std::string,bool> make_header_response();
};

#endif
