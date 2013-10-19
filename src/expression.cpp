#include "expression.h"

inline std::string QueryNode::indent(unsigned int d) {
  return std::string(d * 2, ' ');
}

QueryNode::~QueryNode() {};

QueryTagged::QueryTagged(std::string* string) : QueryNode(), string(string)
{
}

void QueryTagged::print(std::ostream &os, unsigned int depth) const {
  os << indent(depth) << "tagged = " << *string << std::endl;
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


QueryContext::~QueryContext()
{
  clearExpressions();
}

void	QueryContext::clearExpressions()
{
  for(unsigned int i = 0; i < expressions.size(); ++i)
  {
    delete expressions[i];
  }
  expressions.clear();
}
