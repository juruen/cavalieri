#include <boost/variant/get.hpp>
#include <glog/logging.h>
#include <expression.h>
#include <util/util.h>


query_node::~query_node() {};

query_fn_t query_true::evaluate() const {

  return [=](const Event &) {
    return true;
  };

}

query_tagged::query_tagged(std::string * string) : query_node(), string_(string)
{
}

query_fn_t query_tagged::evaluate() const {

  std::string tag(string_->substr(1, string_->size() - 2));

  return [=](const Event & e) {
    return e.has_tag(tag);
  };

}

query_field::query_field(std::string * field, std::string * value,
                         std::string * op)
:
  query_node(),
  op_(op),
  field_(field),
  value_(std::shared_ptr<std::string>(value))
{
}

query_field::query_field(std::string * field, const int value, std::string * op)
:
  query_node(),
  op_(op),
  field_(field),
  value_(value)
{
}

query_field::query_field(std::string * field, const double value,
                         std::string * op)
:
  query_node(),
  op_(op),
  field_(field),
  value_(value)
{
}

query_field::query_field(std::string * field)
:
  query_node(),
  op_(nullptr),
  field_(field),
  value_()
{
}




query_fn_t query_field::evaluate() const {

  if (op_) {

    switch (value_.which()) {

      case 0:
        return evaluate(*(boost::get<std::shared_ptr<std::string>>(value_)));
        break;

      case 1:
        return evaluate(boost::get<int>(value_));
        break;

      case 2:
        return evaluate(boost::get<double>(value_));
        break;

      default:
        VLOG(2) << "query_field::evluate() unknown rvalue";
        return [=](const Event &) { return false; };
        break;

    }

  } else {

    return evaluate_nil();

  }

}

query_fn_t query_field::evaluate_nil() const {

   return [=](const Event & e) { return !e.has_field_set(*field_); };

}

bool  compare(const std::string & left,
              const std::string & right,
              const  std::string & op)
{

  if (op == "=") {

    return left == right;

  }

  if (op == "=~") {

    return match_like(left, right);

  }

  return false;
}

query_fn_t query_field::evaluate(const std::string & value) const {

  const std::string key(*field_);
  std::string strip_val(value);
  std::string op(*op_);

  if (value.size() > 2 && value[0] == '"') {
    strip_val = value.substr(1, value.size() - 2);
  }

  if (*field_ == "host") {

    return [=](const Event & e) { return compare(e.host(), strip_val, op); };

  } else if (*field_ == "service") {

    return [=](const Event & e) { return compare(e.service(), strip_val, op); };

  } else if (*field_ == "state") {

    return [=](const Event & e) { return compare(e.state(), strip_val, op); };

  } else if (*field_ == "description") {

    return [=](const Event & e) {
      return compare(e.description(), strip_val, op);
    };

  } else {

    return [=](const Event & e) {

      if (e.has_attr(key)) {

        return compare(e.attr(key), strip_val, "=");

      }

      return false;
    };
  }
}

query_fn_t query_field::evaluate(const int & value) const {

  const std::string key(*field_);
  const int ival = value;
  const std::string op(*op_);

  if (*field_ == "time") {

    return [=](const Event & e) {
      return compare(e.time(), static_cast<int64_t>(ival), op);
    };

  } else if (*field_ == "ttl") {

    return [=](const Event & e) {
      return compare(static_cast<double>(e.ttl()),
                     static_cast<double>(ival), op);
    };

  } else if (*field_ == "metric") {

    return [=](const Event & e) {

      if (!e.has_metric()) {
        return false;
      }

      return compare(e.metric(), static_cast<double>(ival), op);

    };

  } else {

    return [=](const Event & e) {

      if (e.has_attr(key)) {

        try {

          return compare(std::stoi(e.attr(key)), ival, op);

        } catch (std::invalid_argument &) { }

      }

      return false;
    };
  }

}

query_fn_t query_field::evaluate(const double & value) const {

  const std::string key(*field_);
  const double ival = value;
  const std::string op(*op_);

  if (*field_ == "metric") {

    return [=](const Event & e) {
      return compare(e.metric(), ival, op);
    };

  } else {

    return [=](const Event & e) {

      if (e.has_attr(key)) {

        try {
          return compare(std::stod(e.attr(key)), ival, op);
        } catch (std::invalid_argument &) { }

      }

      return false;
    };

  }
}

query_and::query_and(query_node * left, query_node * right) :
  query_node(),
  left_(left),
  right_(right)
{
}

query_fn_t query_and::evaluate() const {

  query_fn_t left_f = left_->evaluate();
  query_fn_t right_f = right_->evaluate();

  return [=](const Event & e) {
    return (left_f(e) && right_f(e));
  };

}

query_or::query_or(query_node * left, query_node * right) :
  query_node(),
  left_(left),
  right_(right)
{
}

query_fn_t query_or::evaluate() const {
  query_fn_t left_f = left_->evaluate();
  query_fn_t right_f = right_->evaluate();

  return [=](const Event & e) {
    return (left_f(e) || right_f(e));
  };

}

query_not::query_not(query_node * right) :
  query_node(),
  right_(right)
{
}

query_fn_t query_not::evaluate() const {
  query_fn_t right_f = right_->evaluate();

  return [=](const Event & e) {
    return (!right_f(e));
  };

}

void query_context::set_expression(query_node * expression) {
  expression_.reset(expression);
}

query_fn_t query_context::evaluate() {
  return expression_->evaluate();
}
