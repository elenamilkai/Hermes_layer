//
// Created by Yifei Yang on 12/10/21.
//

#include <fpdb/expression/gandiva/If.h>
#include <gandiva/tree_expr_builder.h>

namespace fpdb::expression::gandiva {

If::If(const shared_ptr<Expression> &ifExpr,
       const shared_ptr<Expression> &thenExpr,
       const shared_ptr<Expression> &elseExpr) :
  Expression(IF),
  ifExpr_(ifExpr),
  thenExpr_(thenExpr),
  elseExpr_(elseExpr) {}

void If::compile(const shared_ptr<arrow::Schema> &schema) {
  ifExpr_->compile(schema);
  thenExpr_->compile(schema);
  elseExpr_->compile(schema);

  // FIXME: check return type between "then" and "else"
  returnType_ = thenExpr_->getReturnType();
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeIf(ifExpr_->getGandivaExpression(),
                                                          thenExpr_->getGandivaExpression(),
                                                          elseExpr_->getGandivaExpression(),
                                                          returnType_);
}

string If::alias() {
  return "?column?";
}

string If::getTypeString() {
  return "If";
}

set<string> If::involvedColumnNames() {
  auto ifInvolvedColumnNames = ifExpr_->involvedColumnNames();
  const auto &thenInvolvedColumnNames = thenExpr_->involvedColumnNames();
  const auto &elseInvolvedColumnNames = elseExpr_->involvedColumnNames();
  ifInvolvedColumnNames.insert(thenInvolvedColumnNames.begin(), thenInvolvedColumnNames.end());
  ifInvolvedColumnNames.insert(elseInvolvedColumnNames.begin(), elseInvolvedColumnNames.end());
  return ifInvolvedColumnNames;
}

shared_ptr<Expression> if_(const shared_ptr<Expression> &ifExpr,
                           const shared_ptr<Expression> &thenExpr,
                           const shared_ptr<Expression> &elseExpr) {
  return make_shared<If>(ifExpr, thenExpr, elseExpr);
}

}
