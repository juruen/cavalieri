#ifndef WS_UTIL_H
#define WS_UTIL_H

#include <string>
#include <vector>
#include <unordered_map>


class ws_util {
  public:

    struct header_values {
      std::string method;
      std::string uri;
      std::string version;
      std::unordered_map<std::string, const std::string> headers;

    } header_vals;

    typedef struct {
      bool malformed_header;
      bool pending_bytes;
      size_t header_length;
      uint64_t payload_length;
      std::string decoded;
    } frame_length_t;

    bool find_header_end(const std::string& header);

    bool parse_header(const std::string& header);

    std::pair<std::string,bool> make_header_response();

    std::string create_frame(const std::string payload);

    frame_length_t decode_frame(const std::vector<unsigned char>, size_t);
};

#endif
