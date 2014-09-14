#ifndef CAVALIERI_PREDICATES_PREDICATES_H
#define CAVALIERI_PREDICATES_PREDICATES_H

#define PRED(EXP) [=](e_t e) { return (EXP); }

namespace predicates {

predicate_t above_eq(const double value);

predicate_t above(const double value);

predicate_t under_eq(const double value);

predicate_t under(const double value);

predicate_t state(const std::string state);

predicate_t service(const std::string state);

predicate_t match(const std::string key, const std::string value);

predicate_t match_any(const std::string key,
                      const std::vector<std::string> values);

predicate_t match_re(const std::string key, const std::string value);

predicate_t match_re_any(const std::string key,
                          const std::vector<std::string> values);

predicate_t match_like(const std::string key, const std::string value);

predicate_t match_like_any(const std::string key,
                           const std::vector<std::string> values);

predicate_t default_true();

bool tagged_any(e_t e, const tags_t& tags);

bool tagged_all(e_t e, const tags_t& tags);

bool expired(e_t e);

bool above_eq(e_t e, const double value);

bool above(e_t e, const double value);

bool under_eq(e_t e, const double value);

bool under(e_t e, const double value);

bool match(e_t e, const std::string key, const std::string value);

bool match_re(e_t e, const std::string key, const std::string value);

bool match_like(e_t e, const std::string key, const std::string value);

}

#endif
