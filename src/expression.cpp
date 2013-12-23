#include <expression.h>
#include <util.h>
#include <glog/logging.h>

inline std::string QueryNode::indent(unsigned int d) {
  return std::string(d * 2, ' ');
}

QueryNode::~QueryNode() {};

QueryTrue::QueryTrue() : QueryNode()
{
}

void QueryTrue::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << "true" << std::endl;
}

query_f_t QueryTrue::evaluate() const {
  return [=](const Event& e) {
    UNUSED_VAR(e);
    VLOG(3) << "QueryTrue()::evaluate()";
    return true;
  };
}

QueryTagged::QueryTagged(std::string* string) : QueryNode(), string(string)
{
}

void QueryTagged::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << "tagged = " << *string << std::endl;
}

query_f_t QueryTagged::evaluate() const {
  std::string tag(string->substr(1, string->size() - 2));
  return [=](const Event& e) {
    VLOG(3) << "QueryTagged()::evaluate() tag: " << tag;
    return tag_exists(e, tag);
  };
}

QueryField::QueryField(std::string* field, std::string* value)
:
  QueryNode(),
  field(field),
  value(value)
{
}

void QueryField::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << *field << " = "  << *value << std::endl;
}

query_f_t QueryField::evaluate() const {
  std::string val(*value);

  std::string strip_val;
  if (val.size() > 3) {
    strip_val = val.substr(1, val.size() - 2);
  }

  if (*field == "host") {
    return [=](const Event& e) { return (e.host() == strip_val); };
  } else if (*field == "service") {
    return [=](const Event& e) { return (e.service() == strip_val); };
  } else if (*field == "state") {
    return [=](const Event& e) { return (e.state() == strip_val); };
  } else if (*field == "description") {
    return [=](const Event& e) { return (e.description() == strip_val); };
  } else if (*field == "time") {
    auto ival = std::stoi(val);
    return [=](const Event& e) { return (e.time() == ival); };
  } else if (*field == "ttl") {
    auto ival = std::stoi(val);
    return [=](const Event& e) { return (e.ttl() == ival); };
  } else if (*field == "metric") {
    auto dval = std::stod(val);
    return [=](const Event& e) { return (metric_to_double(e) == dval); };
  } else {
    std::string key(*field);
    return [=](const Event& e)
      {
        for (int i = 0; i < e.attributes_size(); i++) {
          if (e.attributes(i).has_key() &&
              e.attributes(i).key() == key)
          {
            return (e.attributes(i).value() == strip_val);
          }
        }
        return false;
      };
  }
}

QueryAnd::QueryAnd(QueryNode* left, QueryNode* right) :
  QueryNode(),
  left(left),
  right(right)
{
}

void QueryAnd::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << " and" << std::endl;
  left->print(os, depth + 1);
  right->print(os, depth + 1);
}

query_f_t QueryAnd::evaluate() const {
  query_f_t left_f = left->evaluate();
  query_f_t right_f = right->evaluate();
  return [=](const Event& e) {
    VLOG(3) << "QueryAnd()::evaluate()";
    return (left_f(e) && right_f(e));
  };
}

QueryOr::QueryOr(QueryNode* left, QueryNode* right) :
  QueryNode(),
  left(left),
  right(right)
{
}

void QueryOr::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << " or" << std::endl;
  left->print(os, depth + 1);
  right->print(os, depth + 1);
}

query_f_t QueryOr::evaluate() const {
  query_f_t left_f = left->evaluate();
  query_f_t right_f = right->evaluate();
  return [=](const Event& e) {
    VLOG(3) << "QueryOr()::evaluate()";
    return (left_f(e) || right_f(e));
  };
}

QueryNot::QueryNot(QueryNode* right) :
  QueryNode(),
  right(right)
{
}

void QueryNot::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << " not" << std::endl;
  right->print(os, depth + 1);
}

query_f_t QueryNot::evaluate() const {
  query_f_t right_f = right->evaluate();
  return [=](const Event& e) {
    VLOG(3) << "QueryNot()::evaluate()";
    return (!right_f(e));
  };
}


QueryContext::QueryContext() : expression(0) {};

QueryContext::~QueryContext()
{
  clearExpressions();
}

void	QueryContext::clearExpressions()
{
  if (!expression) {
    delete expression;
    expression = 0;
  }
}
