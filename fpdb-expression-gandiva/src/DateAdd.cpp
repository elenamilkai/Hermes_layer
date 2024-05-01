//
// Created by Yifei Yang on 12/10/21.
//

#include <gandiva/tree_expr_builder.h>
#include <fpdb/expression/gandiva/DateAdd.h>
#include <sstream>

namespace fpdb::expression::gandiva {

DateAdd::DateAdd(const shared_ptr<Expression>& left, 
                 const shared_ptr<Expression>& right,
                 DateIntervalType intervalType):
  BinaryExpression(left, right, DATE_ADD),
  intervalType_(intervalType) {
}

void DateAdd::compile(const shared_ptr<arrow::Schema> &schema) {
  left_->compile(schema);
  right_->compile(schema);

  const auto &leftGandivaExpr = left_->getGandivaExpression();
  const auto &rightGandivaExpr = right_->getGandivaExpression();
  returnType_ = arrow::date64();

  string funcName = "timestampadd" + intervalTypeToString(intervalType_);
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeFunction(funcName,
                                                                {leftGandivaExpr, rightGandivaExpr},
                                                                returnType_);
}

string DateAdd::alias() {
  return "?column?";
}

string DateAdd::getTypeString() {
  return "DateAdd-" + intervalTypeToString(intervalType_);
}

shared_ptr<Expression> datePlus(const shared_ptr<Expression>& left,
                                const shared_ptr<Expression>& right,
                                DateIntervalType intervalType) {
  return make_shared<DateAdd>(left, right, intervalType);
}

}
