#ifndef CAVALIERI_UTIL_H
#define CAVALIERI_UTIL_H

#include <common/event.h>
#include <ev++.h>
#include <functional>
#include <vector>
#include <list>
#include <boost/variant.hpp>

#define UNUSED_VAR(x) (void)x

bool match_regex(const std::string value, const std::string re);

bool match_like(const std::string value, const std::string re);

void set_event_value(
    Event & e,
    const std::string & key,
    const std::string & value,
    const bool & replace
);

void set_event_value(
    Event & e,
    const std::string & key,
    const int & value,
    const bool& replace
);

void set_event_value(
    Event & e,
    const std::string & key,
    const double & value,
    const bool & replace
);

void set_event_value(
    Event & e,
    const std::string & key,
    const boost::variant<std::string, int, double> & val,
    const bool & replace
);

std::basic_string<char> base64Encode(std::vector<unsigned char> inputBuffer);

std::string sha1(const std::string& str);

bool parse_uri(
    const std::string& escaped_uri,
    std::string& index,
    std::map<std::string, std::string>& params );

template <class T>
std::vector<T> conj(const std::vector<T> &v, T t) {
  std::vector<T> nv(v);
  nv.push_back(t);
  return std::move(nv);
}

template <class T>
std::list<T> conj(const std::list<T> &v, T t) {
  std::list<T> nv(v);
  nv.push_back(t);
  return std::move(nv);
}

char**  copy_args(int argc, char **argv);

void free_args(int argc, char **argv);

void ld_environment(char **argv, const std::string dir);

#endif

