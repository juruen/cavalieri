#ifndef QUERY_EXPRESSION_H
#define QUERY_EXPRESSION_H

#include <vector>
#include <ostream>
#include <iostream>
#include <memory>
#include <functional>
#include <boost/variant/variant.hpp>
#include <proto.pb.h>

typedef std::function<bool(const Event&)> query_fn_t;

class query_node
{
public:
  virtual query_fn_t evaluate() const = 0;
  virtual ~query_node();
};

class query_true : public query_node
{
public:
  virtual query_fn_t evaluate() const;
};

class query_tagged : public query_node
{
public:
  query_tagged(std::string * string);
  virtual query_fn_t evaluate() const;

private:
  std::unique_ptr<std::string> string_;
};

class query_field : public query_node
{
  public:
  query_field(std::string * field, std::string * value, std::string * op);
  query_field(std::string * field, const int   value, std::string * op);
  query_field(std::string * field, const double  value, std::string * op);
  query_field(std::string * field);
  query_fn_t evaluate() const;

private:

  query_fn_t evaluate(const std::string & value) const;
  query_fn_t evaluate(const int & value) const;
  query_fn_t evaluate(const double & value) const;
  query_fn_t evaluate_nil() const;

  std::unique_ptr<std::string> op_;
  std::unique_ptr<std::string> field_;
  boost::variant<std::shared_ptr<std::string>, int, double> value_;
};

class query_and : public query_node
{
public:
 query_and(query_node * left, query_node * right);
 query_fn_t evaluate() const;

private:
  std::unique_ptr<query_node> left_;
  std::unique_ptr<query_node> right_;
};

class query_or : public query_node
{
public:
  query_or(query_node * left, query_node * right);
  query_fn_t evaluate() const;

private:
  std::unique_ptr<query_node> left_;
  std::unique_ptr<query_node> right_;
};

class query_not : public query_node
{
public:
  query_not(query_node* right);
  query_fn_t evaluate() const;

private:
 std::unique_ptr<query_node> right_;
};


class query_context
{
public:
  void set_expression(query_node * expression);
  query_fn_t evaluate();

private:
 std::unique_ptr<query_node> expression_;

};

template<typename T>
bool compare(const T & left, const T & right, const std::string & op) {

  if (op == "=") {
    return left == right;
  }

  if (op == ">") {
    return left > right;
  }

  if (op == ">=") {
    return left >= right;
  }

  if (op == "<") {
    return left < right;
  }

  if (op == "<=") {
    return left <= right;
  }

  return false;
}

#endif
