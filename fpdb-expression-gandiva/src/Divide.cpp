//
// Created by matt on 28/4/20.
//

#include <gandiva/tree_expr_builder.h>

#include <utility>
#include "fpdb/expression/gandiva/Divide.h"

using namespace fpdb::expression::gandiva;

Divide::Divide(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
	: BinaryExpression(std::move(left), std::move(right), DIVIDE) {}

void Divide::compile(const std::shared_ptr<arrow::Schema> &schema) {
  left_->compile(schema);
  right_->compile(schema);

  const auto &castRes = castGandivaExprToUpperType();
  const auto &leftGandivaExpr = get<1>(castRes);
  const auto &rightGandivaExpr = get<2>(castRes);

  returnType_ = get<0>(castRes);
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeFunction("divide",
                                                                {leftGandivaExpr, rightGandivaExpr},
                                                                returnType_);
}

std::string Divide::alias() {
  return "?column?";
}

std::string Divide::getTypeString() {
  return "Divide";
}

std::shared_ptr<Expression> fpdb::expression::gandiva::divide(const std::shared_ptr<Expression>& left,
																const std::shared_ptr<Expression>& right) {
  return std::make_shared<Divide>(left, right);
}
