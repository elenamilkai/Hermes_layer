//
// Created by matt on 11/6/20.
//

#include <fpdb/expression/gandiva/And.h>
#include <gandiva/tree_expr_builder.h>
#include <sstream>

using namespace fpdb::expression::gandiva;

And::And(const vector<shared_ptr<Expression>>& exprs): 
  Expression(AND),
  exprs_(exprs) {
}

void And::compile(const shared_ptr<arrow::Schema> &schema) {
  ::gandiva::NodeVector gandivaExprs;
  for (const auto &expr: exprs_) {
    expr->compile(schema);
    gandivaExprs.emplace_back(expr->getGandivaExpression());
  }

  returnType_ = ::arrow::boolean();
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeAnd(gandivaExprs);
}

string And::alias() {
  stringstream ss;
  ss << "(";
  for (uint i = 0; i < exprs_.size(); ++i) {
    ss << exprs_[i]->alias();
    if (i < exprs_.size() - 1) {
      ss << " and ";
    }
  }
  ss << ")";
  return ss.str();
}

string And::getTypeString() {
  return "And";
}

set<string> And::involvedColumnNames() {
  set<string> allInvolvedColumnNames;
  for (const auto &expr: exprs_) {
    const auto &involvedColumnNames = expr->involvedColumnNames();
    allInvolvedColumnNames.insert(involvedColumnNames.begin(), involvedColumnNames.end());
  }
  return allInvolvedColumnNames;
}

const vector<shared_ptr<Expression>> &And::getExprs() const {
  return exprs_;
}

shared_ptr<Expression> fpdb::expression::gandiva::and_(const shared_ptr<Expression>& left,
															  const shared_ptr<Expression>& right) {
  const vector<shared_ptr<Expression>> exprs{left, right};
  return make_shared<And>(exprs);
}

shared_ptr<Expression>
fpdb::expression::gandiva::and_(const vector<shared_ptr<Expression>> &exprs) {
  return make_shared<And>(exprs);
}
