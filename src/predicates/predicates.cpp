#include <string>
#include <vector>
#include <core/core.h>
#include <util/util.h>
#include <predicates/predicates.h>

namespace predicates {

const unsigned int k_default_ttl = 60;

predicate_t above_eq(const double value) {
  return PRED(above_eq(e, value));
}

predicate_t above(const double value) {
  return PRED(above(e, value));
}

predicate_t under_eq(const double value) {
  return PRED(under_eq(e, value));
}

predicate_t under(const double value) {
  return PRED(under(e, value));
}

predicate_t state(const std::string state) {
  return PRED(e.state() == state);
}

predicate_t service(const std::string service) {
  return PRED(e.service() == service);
}

predicate_t match(const std::string key, const std::string value) {
  return PRED(match(e, key, value));
}

predicate_t match_any(const std::string key,
                      const std::vector<std::string> values)
{
  return PRED(
      std::find(values.begin(), values.end(), e.value_to_str(key))
      != values.end()
  );
}

predicate_t match_re(const std::string key, const std::string value) {
  return PRED(match_re(e, key, value));
}

predicate_t match_re_any(const std::string key,
                         const std::vector<std::string> values)
{
  return [=](e_t e) {

    for (const auto & val : values) {
      if (match_re(e, key, val)) {
        return true;
      }
    }

    return false;
  };
}

predicate_t match_like(const std::string key, const std::string value) {
  return PRED(match_like(e, key, value));
}

predicate_t match_like_any(const std::string key,
                           const std::vector<std::string> values)
{
  return [=](e_t e) {

    for (const auto & val : values) {
      if (match_like(e, key, val)) {
        return true;
      }
    }

    return false;
  };
}

predicate_t default_true() {
  return [](e_t){ return true; };
}

bool tagged_any(e_t e, const tags_t& tags) {
  for (auto &tag: tags) {
    if (e.has_tag(tag)) {
      return true;
    }
  }
  return false;
}

bool tagged_all(e_t e, const tags_t& tags) {
  for (auto &tag: tags) {
    if (!e.has_tag(tag)) {
      return false;
    }
  }
  return true;
}

bool expired(e_t e) {
  auto ttl = e.has_ttl() ? e.ttl() : k_default_ttl;

  if (e.state() == "expired") {
    return true;
  }

  if (g_core->sched().unix_time() < e.time()) {
    return false;
  }

  return (g_core->sched().unix_time() - e.time() > ttl);
}

bool above_eq(e_t e, const double value) {
  return (e.metric() >= value);
}

bool above(e_t e, const double value) {
  return (e.metric() > value);
}

bool under_eq(e_t e, const double value) {
  return (e.metric() <= value);
}

bool under(e_t e, const double value) {
  return (e.metric() < value);
}

bool match(e_t e, const std::string key, const std::string value) {

 return e.value_to_str(key) == value;

}

bool match_re(e_t e, const std::string key, const std::string value) {

  return match_regex(e.value_to_str(key), value);

}

bool match_like(e_t e, const std::string key, const std::string value) {

  return ::match_like(e.value_to_str(key), value);

}

}
