#include "util.h"
#include <string>
#include <sstream>
#include <glog/logging.h>
#include <openssl/sha.h>
#include <curl/curl.h>

namespace {
  const uint32_t buff_size = 1024;
  const uint32_t json_buff_size = 1024 * 4;
  const char *json_base =
      "{\"host\": \"%s\", \"service\": \"%s\", \"description\": \"%s\""
      ",\"state\": \"%s\", \"metric\": %f, \"tags\": [%s] %s}";
}

std::string metric_to_string(const Event& e) {
  std::ostringstream ss;
  if (e.has_metric_f()) {
    ss << e.metric_f();
  } else if (e.has_metric_d()) {
    ss << e.metric_d();
  } else if (e.has_metric_sint64()) {
    ss << e.metric_sint64();
  } else {
    ss << "";
  }
  return ss.str();
}

std::string ttl_to_string(const Event& e) {
  std::ostringstream ss;
  ss << e.ttl();
  return ss.str();
}

double metric_to_double(const Event &e) {
  if (e.has_metric_f()) {
    return e.metric_f();
  } else if (e.has_metric_sint64()) {
    return e.metric_sint64();
  } else if (e.has_metric_d()) {
    return e.metric_d();
  } else {
    return 0;
  }
}


std::string string_to_value(const Event& e, const std::string& key) {
  if (key == "host") {
    return e.host();
  } else if (key == "service") {
    return e.service();
  } else if (key == "description") {
    return e.description();
  } else if (key == "state") {
    return e.state();
  } else if (key == "metric") {
    return metric_to_string(e);
  }  else if (key == "ttl") {
    return ttl_to_string(e);
  } else {
    return "__nil__";
  }
}

std::string event_to_json(const Event &e) {
  char tags[buff_size] =  "";
  char attrs[buff_size] = "";

  for (int i = 0; i < e.tags_size(); i++) {
    if (i != 0) {
      strncat(tags, ", ", buff_size- strlen(tags));
    }
    strncat(tags, "\"", buff_size - strlen(tags));
    strncat(tags, e.tags(i).c_str(), buff_size - strlen(tags));
    strncat(tags, "\"", buff_size - strlen(tags));
  }

  for (int i = 0; i < e.attributes_size(); i++) {
    strncat(attrs, ", ", buff_size - strlen(attrs));
    strncat(attrs, "\"", buff_size - strlen(attrs));
    strncat(attrs, e.attributes(i).key().c_str(), buff_size - strlen(attrs));
    strncat(attrs, "\": ", buff_size - strlen(attrs));
    strncat(attrs, "\"", buff_size - strlen(attrs));
    strncat(attrs, e.attributes(i).value().c_str(), buff_size - strlen(attrs));
    strncat(attrs, "\"", buff_size - strlen(attrs));
  }

 double metric;
 if (e.has_metric_f()) {
    metric = (double)e.metric_f();
  } else if (e.has_metric_d()) {
    metric =  e.metric_d();
  } else if (e.has_metric_sint64()) {
    metric = (double) e.metric_sint64();
  } else {
    metric = 0;
  }

  char json_buffer[json_buff_size];
  size_t r = snprintf(json_buffer, json_buff_size, json_base,
                e.host().c_str(), e.service().c_str(), e.description().c_str(),
                e.state().c_str(), metric, tags, attrs);

  if (r >= json_buff_size) {
    VLOG(1) << "json string is too big";
    return "";
  } else {
    return json_buffer;
  }
}

void set_event_value(
    Event& e,
    const std::string& key,
    const std::string& value,
    const bool& replace)
{
  if (key == "host") {
    if (replace || (!e.has_host())) {
      e.set_host(value);
    }
  } else if (key == "service") {
    if (replace || (!e.has_service())) {
      e.set_service(value);
    }
  } else if (key == "description") {
    if (replace || (!e.has_description())) {
      e.set_description(value);
    }
  } else if (key == "state") {
    if (replace || (!e.has_state())) {
      e.set_state(value);
    }
  } else if (key == "metric") {
    if (e.has_metric_d() && replace) {
      e.set_metric_d(atof(value.c_str()));
    } else if (e.has_metric_sint64() && replace) {
      e.set_metric_sint64(atoi(value.c_str()));
    } else {
      if (replace || !e.has_metric_f()) {
        e.set_metric_f(atof(value.c_str()));
      }
    }
  } else if (key == "ttl") {
    if (replace || (!e.has_ttl())) {
      e.set_ttl(atof(value.c_str()));
    }
  } else {
    LOG(ERROR) << "string_to_value() wrong key: " << key;
  }
}

bool tag_exists(const Event& e, const std::string& tag) {
  for (int i = 0; i < e.tags_size(); i++) {
    if (e.tags(i) == tag) {
      return true;
    }
  }
  return false;
}

bool attribute_exists(const Event& e, const std::string& attribute) {
  for (int i = 0; i < e.attributes_size(); i++) {
    if (e.attributes(i).key() == attribute) {
      return true;
    }
  }
  return false;
}

#include <iostream>
std::string attribute_value(const Event& e, const std::string& attribute) {
  if (attribute_exists(e, attribute)) {
    for (int i = 0; i < e.attributes_size(); i++) {
      if (e.attributes(i).key() == attribute) {
        return e.attributes(i).value();
      }
    }
  }
  return "";
}

std::string sha1(const std::string& str) {
  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1((const unsigned char*)str.c_str(), (unsigned long)str.size(), hash);
  return std::string((char*)hash, SHA_DIGEST_LENGTH);
}

/* Taken and slightly modified from
   http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64 */

const static char encodeLookup[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const static char padCharacter = '=';
std::basic_string<char> base64Encode(std::vector<unsigned char> inputBuffer)
{
  std::basic_string<char> encodedString;
  encodedString.reserve(((inputBuffer.size()/3) + (inputBuffer.size() % 3 > 0)) * 4);
  long temp;
  std::vector<unsigned char>::iterator cursor = inputBuffer.begin();
  for(size_t idx = 0; idx < inputBuffer.size()/3; idx++)
  {
    temp  = (*cursor++) << 16; //Convert to big endian
    temp += (*cursor++) << 8;
    temp += (*cursor++);
    encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
    encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
    encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
    encodedString.append(1,encodeLookup[(temp & 0x0000003F)      ]);
  }
  switch(inputBuffer.size() % 3)
  {
    case 1:
      temp  = (*cursor++) << 16; //Convert to big endian
      encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
      encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
      encodedString.append(2,padCharacter);
      break;
    case 2:
      temp  = (*cursor++) << 16; //Convert to big endian
      temp += (*cursor++) << 8;
      encodedString.append(1,encodeLookup[(temp & 0x00FC0000) >> 18]);
      encodedString.append(1,encodeLookup[(temp & 0x0003F000) >> 12]);
      encodedString.append(1,encodeLookup[(temp & 0x00000FC0) >> 6 ]);
      encodedString.append(1,padCharacter);
      break;
  }
  return encodedString;
}

std::string uri_unescape(const std::string& uri) {
  VLOG(3) << "uri_unescape() uri: " << uri;
  CURL *curl = curl_easy_init();

  if (curl == NULL) {
    LOG(ERROR) << "uri_unescape(): curl_easy_init() failed";
    return uri;
  }

  char *ret =  curl_easy_unescape(curl, uri.c_str(), uri.size(), 0);
  VLOG(3) << "*ret " << ret;
  const std::string unescaped(ret);
  curl_free(ret);
  curl_easy_cleanup(curl);
  return unescaped;
}

bool parse_uri(
    const std::string& escaped_uri,
    std::string& index,
    std::map<std::string, std::string>& params )
{
  VLOG(3) << "parse_uri() escaped_uri: " << escaped_uri;
  std::string uri = uri_unescape(escaped_uri);
  VLOG(3) << "parse_uri() uri: " << uri;

  auto it = uri.begin();
  if (*it != '/') {
    LOG(ERROR) << "uri doesn't start with /";
    return false;
  }

  while (it != uri.end() && *it != '?') {
    index += *it;
    it++;
  }

  if (it == uri.end()) {
    LOG(ERROR) << "uri doesn't contain '?'";
    return false;
  }
  it++;

  VLOG(3) << "index name: " << index;

  while(true) {
    std::string key;
    while (it != uri.end() && *it != '=') {
      key += *it;
      it++;
    }

    if (it == uri.end()) {
      LOG(ERROR) << "uri doesn't contain '='";
      return false;
    }
    it++;

    VLOG(3) << "key: " << key;

    std::string value;
    while (it != uri.end() && *it != '&') {
      value += *it;
      it++;
    }

    VLOG(3) << "value: " << value;
    params.insert({key, value});

    if (it == uri.end()) {
      break;
    } else if (*it == '&') {
      it++;
    }
  }

  return true;
}


