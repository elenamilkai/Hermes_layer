//
// Created by Yifei Yang on 12/10/21.
//

#include <fpdb/expression/gandiva/Not.h>
#include <gandiva/tree_expr_builder.h>

namespace fpdb::expression::gandiva {

Not::Not(const shared_ptr<Expression> &expr):
  Expression(NOT),
  expr_(expr) {}

void Not::compile(const shared_ptr<arrow::Schema> &schema) {
  expr_->compile(schema);
  returnType_ = ::arrow::boolean();
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeFunction("not",
                                                                {expr_->getGandivaExpression()},
                                                                returnType_);
}

string Not::alias() {
  return "not (" + expr_->alias() + ")";
}

string Not::getTypeString() {
  return "Not";
}

set<string> Not::involvedColumnNames() {
  return expr_->involvedColumnNames();
}

shared_ptr<Expression> not_(const shared_ptr<Expression> &expr) {
  return make_shared<Not>(expr);
}

}
