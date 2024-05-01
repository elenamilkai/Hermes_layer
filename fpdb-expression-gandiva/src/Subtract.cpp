//
// Created by matt on 28/4/20.
//

#include <gandiva/tree_expr_builder.h>

#include <utility>
#include "fpdb/expression/gandiva/Subtract.h"

using namespace fpdb::expression::gandiva;

Subtract::Subtract(std::shared_ptr<Expression> left, std::shared_ptr<Expression> right) :
	BinaryExpression(std::move(left), std::move(right), SUBTRACT) {}

void Subtract::compile(const std::shared_ptr<arrow::Schema> &schema) {
  left_->compile(schema);
  right_->compile(schema);

  const auto &castRes = castGandivaExprToUpperType();
  const auto &leftGandivaExpr = get<1>(castRes);
  const auto &rightGandivaExpr = get<2>(castRes);

  returnType_ = get<0>(castRes);
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeFunction("subtract",
                                                                {leftGandivaExpr, rightGandivaExpr},
                                                                returnType_);
}

std::string Subtract::alias() {
  return "?column?";
}

std::string Subtract::getTypeString() {
  return "Subtract";
}

std::shared_ptr<Expression> fpdb::expression::gandiva::minus(const std::shared_ptr<Expression>& left,
															   const std::shared_ptr<Expression>& right) {
  return std::make_shared<Subtract>(left, right);
}
