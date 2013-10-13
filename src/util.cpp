#include "util.h"
#include <string>
#include <sstream>
#include <glog/logging.h>
#include <openssl/sha.h>


CallbackTimer::CallbackTimer(const int interval, const std::function<void()> f)
  : f_(f)
{
      tio_.set<CallbackTimer, &CallbackTimer::callback>(this);
      tio_.start(0, interval);
}

void CallbackTimer::callback() {
  f_();
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
  //XXX Use std::string instead of ostringstream.
  //    This is slow as fuck and not even necessary.
  std::ostringstream tags;
  tags << "[ ";
  for (int i = 0; i < e.tags_size(); i++) {
    if (i != 0) {
      tags << ", "; // I miss ",".join() :(
    }
    tags << "\"" << e.tags(i) << "\"";

  }
  tags << " ]";

  std::ostringstream attrs;
  for (int i = 0; i < e.attributes_size(); i++) {
    if (i != 0) {
      attrs << ", ";
    }
    attrs << "\"" << e.attributes(i).key() << "\": ";
    attrs << "\"" << e.attributes(i).value() << "\"";
  }

  std::ostringstream json;
  json << "{ ";

  json << "\"host\": " << "\"" << e.host() << "\", ";
  json << "\"service\": " << "\"" << e.service() << "\", ";
  json << "\"description\": " << "\"" << e.description() << "\", ";
  json << "\"state\": " << "\"" << e.state() << "\", ";
  json << "\"metric\": " << metric_to_string(e) << " , ";
  json << "\"ttl\": " << e.ttl() << " , ";
  json << "\"tags\": " << tags.str();

  if (e.attributes_size() > 0) {
    json << ", " << attrs.str();
  }

  json << " }";

  return json.str();
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
