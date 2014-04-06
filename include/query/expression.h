#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>
#include <ostream>
#include <iostream>
#include <memory>
#include <functional>
#include <boost/variant/variant.hpp>
#include <proto.pb.h>

typedef std::function<bool(const Event&)> query_f_t;

class QueryNode
{
public:
  virtual void print(std::ostream &os, unsigned int depth=0) const = 0;
  static inline std::string indent(unsigned int d);
  virtual query_f_t evaluate() const = 0;
  virtual ~QueryNode();
};

class QueryTrue : public QueryNode
{
public:
  QueryTrue();
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;
};


class QueryTagged : public QueryNode
{
  std::unique_ptr<std::string> string;

public:
  QueryTagged(std::string* string);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;
};

class QueryField : public QueryNode
{
  std::unique_ptr<std::string> op;
  std::unique_ptr<std::string> field;
  boost::variant<std::shared_ptr<std::string>, int, double> value;

public:
  QueryField(std::string* field, std::string* value, std::string* op);
  QueryField(std::string* field, const int &  value, std::string* op);
  QueryField(std::string* field, const double & value, std::string* op);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;

private:
  query_f_t evaluate(const std::string & value) const;
  query_f_t evaluate(const int & value) const;
  query_f_t evaluate(const double & value) const;
};

class QueryAnd : public QueryNode
{
  std::unique_ptr<QueryNode> left;
  std::unique_ptr<QueryNode> right;

public:
 QueryAnd(QueryNode* left, QueryNode* right);
 virtual void print(std::ostream &os, unsigned int depth=0) const;
 virtual query_f_t evaluate() const;
};

class QueryOr : public QueryNode
{
  std::unique_ptr<QueryNode> left;
  std::unique_ptr<QueryNode> right;

public:
  QueryOr(QueryNode* left, QueryNode* right);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;
};

class QueryNot : public QueryNode
{
  std::unique_ptr<QueryNode> right;

public:
  QueryNot(QueryNode* right);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;
};


class QueryContext
{
  public:
    std::unique_ptr<QueryNode> expression;

     QueryContext();
    ~QueryContext();
    void	clearExpressions();
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
