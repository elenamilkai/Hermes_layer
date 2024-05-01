//
// Created by Yifei Yang on 1/7/22.
//

#include <fpdb/expression/gandiva/IsNull.h>
#include <gandiva/tree_expr_builder.h>

namespace fpdb::expression::gandiva {

IsNull::IsNull(const shared_ptr<Expression> &expr):
  Expression(IS_NULL),
  expr_(expr) {}

void IsNull::compile(const shared_ptr<arrow::Schema> &schema) {
  expr_->compile(schema);
  returnType_ = ::arrow::boolean();
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeFunction("isnull",
                                                                {expr_->getGandivaExpression()},
                                                                returnType_);
}

string IsNull::alias() {
  return expr_->alias() + "is null";
}

string IsNull::getTypeString() {
  return "IsNull";
}

set<string> IsNull::involvedColumnNames() {
  return expr_->involvedColumnNames();
}

shared_ptr<Expression> isNull(const shared_ptr<Expression> &expr) {
  return make_shared<IsNull>(expr);
}

}
