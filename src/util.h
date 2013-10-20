#ifndef UTIL_H
#define UTIL_H

#include "proto.pb.h"
#include <ev++.h>
#include <functional>

#define UNUSED_VAR(x) (void)x

class CallbackTimer {
  private:
    ev::timer tio_;
    std::function<void()> f_;
  public:
    CallbackTimer(const int interval, const std::function<void()> f);
    void callback();
};

std::string metric_to_string(const Event& e);

double metric_to_double(const Event &e);

std::string string_to_value(const Event& e, const std::string& key);

std::string event_to_json(const Event &e);

bool tag_exists(const Event& e, const std::string& tag);

void set_event_value(
    Event& e,
    const std::string& key,
    const std::string& value,
    const bool& replace
);

std::basic_string<char> base64Encode(std::vector<unsigned char> inputBuffer);

std::string sha1(const std::string& str);

bool parse_uri(
    const std::string& escaped_uri,
    std::string& index,
    std::map<std::string, std::string>& params );

#endif

