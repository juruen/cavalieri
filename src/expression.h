#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <map>
#include <vector>
#include <ostream>
#include <stdexcept>
#include <cmath>
#include <iostream>

class QueryNode
{
  public:
    virtual void print(std::ostream &os, unsigned int depth=0) const = 0;
    static inline std::string indent(unsigned int d) {
      return std::string(d * 2, ' ');
    }
    virtual ~QueryNode() {};
};

class QueryTagged : public QueryNode
{
  std::string* string;

public:
  QueryTagged(std::string* string) : QueryNode(), string(string)
  {
  }
  virtual void print(std::ostream &os, unsigned int depth=0) const {
    os << indent(depth) << "tagged = " << *string << std::endl;
  }
};

class QueryAnd : public QueryNode
{
  QueryNode* left;
  QueryNode* right;

public:
  QueryAnd(QueryNode* left, QueryNode* right) :
    QueryNode(),
    left(left),
    right(right)
  {
  }

  virtual void print(std::ostream &os, unsigned int depth=0) const {
    os << indent(depth) << " and" << std::endl;
    left->print(os, depth + 1);
    right->print(os, depth + 1);
  }

};

class QueryOr : public QueryNode
{
  QueryNode* left;
  QueryNode* right;

public:
  QueryOr(QueryNode* left, QueryNode* right) :
    QueryNode(),
    left(left),
    right(right)
  {
  }

  virtual void print(std::ostream &os, unsigned int depth=0) const {
    os << indent(depth) << " or" << std::endl;
    left->print(os, depth + 1);
    right->print(os, depth + 1);
  }

};

class QueryContext
{
  public:

    typedef std::map<std::string, double> variablemap_type;

    variablemap_type		variables;

    std::vector<QueryNode*> expressions;

    ~QueryContext()
    {
      clearExpressions();
    }

    void	clearExpressions()
    {
      for(unsigned int i = 0; i < expressions.size(); ++i)
      {
        delete expressions[i];
      }
      expressions.clear();
    }

    bool	existsVariable(const std::string &varname) const
    {
      return variables.find(varname) != variables.end();
    }

    double	getVariable(const std::string &varname) const
    {
      variablemap_type::const_iterator vi = variables.find(varname);
      if (vi == variables.end())
        throw(std::runtime_error("Unknown variable."));
      else
        return vi->second;
    }
};

#endif 
