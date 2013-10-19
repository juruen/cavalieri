#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <map>
#include <vector>
#include <ostream>
#include <iostream>

class QueryNode
{
  public:
    virtual void print(std::ostream &os, unsigned int depth=0) const = 0;
    static inline std::string indent(unsigned int d);
    virtual ~QueryNode();
};

class QueryTagged : public QueryNode
{
  std::string* string;

public:
  QueryTagged(std::string* string);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
};

class QueryAnd : public QueryNode
{
  QueryNode* left;
  QueryNode* right;

public:
 QueryAnd(QueryNode* left, QueryNode* right);
 virtual void print(std::ostream &os, unsigned int depth=0) const;

};

class QueryOr : public QueryNode
{
  QueryNode* left;
  QueryNode* right;

public:
  QueryOr(QueryNode* left, QueryNode* right);
  virtual void print(std::ostream &os, unsigned int depth=0) const;
};

class QueryContext
{
  public:

    typedef std::map<std::string, double> variablemap_type;
    variablemap_type		variables;
    std::vector<QueryNode*> expressions;

    ~QueryContext();
    void	clearExpressions();
};

#endif
