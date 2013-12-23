#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <vector>
#include <ostream>
#include <functional>
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
  std::string* string;

public:
  QueryTagged(std::string* string);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;
};

class QueryField : public QueryNode
{
  std::string* field;
  std::string* value;

public:
  QueryField(std::string* field, std::string* value);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;
};

class QueryAnd : public QueryNode
{
  QueryNode* left;
  QueryNode* right;

public:
 QueryAnd(QueryNode* left, QueryNode* right);
 virtual void print(std::ostream &os, unsigned int depth=0) const;
 virtual query_f_t evaluate() const;
};

class QueryOr : public QueryNode
{
  QueryNode* left;
  QueryNode* right;

public:
  QueryOr(QueryNode* left, QueryNode* right);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;
};

class QueryNot : public QueryNode
{
  QueryNode* right;

public:
  QueryNot(QueryNode* right);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
  virtual query_f_t evaluate() const;
};


class QueryContext
{
  public:
    QueryNode* expression;

     QueryContext();
    ~QueryContext();
    void	clearExpressions();
};

#endif
