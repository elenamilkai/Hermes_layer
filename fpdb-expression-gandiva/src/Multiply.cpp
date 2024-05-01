//
// Created by matt on 28/4/20.
//

#include "fpdb/expression/gandiva/Multiply.h"

#include <gandiva/tree_expr_builder.h>

#include <utility>

using namespace fpdb::expression::gandiva;

Multiply::Multiply(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
	: BinaryExpression(std::move(left), std::move(right), MULTIPLY) {}

void Multiply::compile(const std::shared_ptr<arrow::Schema> &schema) {
  left_->compile(schema);
  right_->compile(schema);

  const auto &castRes = castGandivaExprToUpperType();
  const auto &leftGandivaExpr = get<1>(castRes);
  const auto &rightGandivaExpr = get<2>(castRes);

  returnType_ = get<0>(castRes);
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeFunction("multiply",
                                                                {leftGandivaExpr, rightGandivaExpr},
                                                                returnType_);
}

std::string Multiply::alias() {
  return "?column?";
}

std::string Multiply::getTypeString() {
  return "Multiply";
}

std::shared_ptr<Expression> fpdb::expression::gandiva::times(const std::shared_ptr<Expression>& left,
															   const std::shared_ptr<Expression>& right) {
  return std::make_shared<Multiply>(left, right);
}

