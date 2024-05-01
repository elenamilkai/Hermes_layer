//
// Created by Yifei Yang on 1/8/22.
//

#include <fpdb/expression/gandiva/Substr.h>
#include <gandiva/tree_expr_builder.h>

namespace fpdb::expression::gandiva {

Substr::Substr(const shared_ptr<Expression> &expr,
               const shared_ptr<Expression> &fromLit,
               const shared_ptr<Expression> &forLit) :
  Expression(SUBSTR),
  expr_(expr),
  fromLit_(fromLit),
  forLit_(forLit) {}

void Substr::compile(const shared_ptr<arrow::Schema> &schema) {
  expr_->compile(schema);
  fromLit_->compile(schema);
  forLit_->compile(schema);

  returnType_ = arrow::utf8();
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeFunction(
          "substr",
          {expr_->getGandivaExpression(), fromLit_->getGandivaExpression(), forLit_->getGandivaExpression()},
          returnType_);
}

string Substr::alias() {
  return "?column?";
}

string Substr::getTypeString() {
  return "SubStr";
}

set<string> Substr::involvedColumnNames() {
  auto involvedColumnNames = expr_->involvedColumnNames();
  const auto &fromInvolvedColumnNames = fromLit_->involvedColumnNames();
  const auto &forInvolvedColumnNames = forLit_->involvedColumnNames();
  involvedColumnNames.insert(fromInvolvedColumnNames.begin(), fromInvolvedColumnNames.end());
  involvedColumnNames.insert(forInvolvedColumnNames.begin(), forInvolvedColumnNames.end());
  return involvedColumnNames;
}

shared_ptr<Expression> substr(const shared_ptr<Expression> &expr,
                              const shared_ptr<Expression> &fromLit,
                              const shared_ptr<Expression> &forLit) {
  return make_shared<Substr>(expr, fromLit, forLit);
}

}
