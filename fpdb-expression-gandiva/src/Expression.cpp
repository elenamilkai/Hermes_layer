//
// Created by matt on 27/4/20.
//

#include <fpdb/expression/gandiva/Expression.h>
#include <fpdb/expression/gandiva/And.h>
#include <fpdb/expression/gandiva/Or.h>
#include <fpdb/expression/gandiva/EqualTo.h>
#include <fpdb/expression/gandiva/Cast.h>
#include <fpdb/expression/gandiva/GreaterThan.h>
#include <fpdb/expression/gandiva/GreaterThanOrEqualTo.h>
#include <fpdb/expression/gandiva/LessThan.h>
#include <fpdb/expression/gandiva/LessThanOrEqualTo.h>
#include <fpdb/util/Util.h>

using namespace fpdb::expression::gandiva;
using namespace fpdb::util;

Expression::Expression(ExpressionType type) :
  type_(type) {}

ExpressionType Expression::getType() const {
  return type_;
}

const gandiva::NodePtr &Expression::getGandivaExpression() const {
  return gandivaExpression_;
}

std::string Expression::showString() {
  return gandivaExpression_->ToString();
}

std::shared_ptr<std::string> fpdb::expression::gandiva::removePrefixInt(const std::string& str) {
  if (str.substr(0, prefixInt_.size()) == prefixInt_) {
    return std::make_shared<std::string>(str.substr(prefixInt_.size(), str.size() - prefixInt_.size()));
  } else {
    return nullptr;
  }
}

std::shared_ptr<std::string> fpdb::expression::gandiva::removePrefixFloat(const std::string& str) {
  if (str.substr(0, prefixFloat_.size()) == prefixFloat_) {
    return std::make_shared<std::string>(str.substr(prefixFloat_.size(), str.size() - prefixFloat_.size()));
  } else {
    return nullptr;
  }
}

std::shared_ptr<arrow::DataType> getNumericType(const std::shared_ptr<fpdb::expression::gandiva::Expression>& expr) {
  if (expr->getTypeString() == "NumericLiteral<Int32>") {
    return arrow::int32();
  } else if (expr->getTypeString() == "NumericLiteral<Int64>") {
    return arrow::int64();
  } else if (expr->getTypeString() == "NumericLiteral<Double>") {
    return arrow::float64();
  } else {
    return nullptr;
  }
}

std::shared_ptr<fpdb::expression::gandiva::Expression> 
fpdb::expression::gandiva::cascadeCast(std::shared_ptr<fpdb::expression::gandiva::Expression> expr) {
  if (expr->getType() == AND) {
    auto andExpr = std::static_pointer_cast<fpdb::expression::gandiva::And>(expr);
    vector<std::shared_ptr<Expression>> castChildExprs;
    for (const auto &childExpr: andExpr->getExprs()) {
      castChildExprs.emplace_back(cascadeCast(childExpr));
    }
    return and_(castChildExprs);
  }

  else if (expr->getType() == OR) {
    auto orExpr = std::static_pointer_cast<fpdb::expression::gandiva::Or>(expr);
    vector<std::shared_ptr<Expression>> castChildExprs;
    for (const auto &childExpr: orExpr->getExprs()) {
      castChildExprs.emplace_back(cascadeCast(childExpr));
    }
    return or_(castChildExprs);
  }

  else if (expr->getType() == EQUAL_TO) {
    auto eqExpr = std::static_pointer_cast<fpdb::expression::gandiva::EqualTo>(expr);
    auto leftExpr = eqExpr->getLeft();
    auto rightExpr = eqExpr->getRight();
    auto leftType = getNumericType(leftExpr);
    if (leftType) {
      return eq(leftExpr, fpdb::expression::gandiva::cast(rightExpr, leftType));
    } else {
      auto rightType = getNumericType(rightExpr);
      if (rightType) {
        return eq(fpdb::expression::gandiva::cast(leftExpr, rightType), rightExpr);
      }
    }
    return expr;
  }

  else if (expr->getType() == GREATER_THAN) {
    auto gtExpr = std::static_pointer_cast<fpdb::expression::gandiva::GreaterThan>(expr);
    auto leftExpr = gtExpr->getLeft();
    auto rightExpr = gtExpr->getRight();
    auto leftType = getNumericType(leftExpr);
    if (leftType) {
      return gt(leftExpr, fpdb::expression::gandiva::cast(rightExpr, leftType));
    } else {
      auto rightType = getNumericType(rightExpr);
      if (rightType) {
        return gt(fpdb::expression::gandiva::cast(leftExpr, rightType), rightExpr);
      }
    }
    return expr;
  }

  else if (expr->getType() == GREATER_THAN_OR_EQUAL_TO) {
    auto geExpr = std::static_pointer_cast<fpdb::expression::gandiva::GreaterThanOrEqualTo>(expr);
    auto leftExpr = geExpr->getLeft();
    auto rightExpr = geExpr->getRight();
    auto leftType = getNumericType(leftExpr);
    if (leftType) {
      return gte(leftExpr, fpdb::expression::gandiva::cast(rightExpr, leftType));
    } else {
      auto rightType = getNumericType(rightExpr);
      if (rightType) {
        return gte(fpdb::expression::gandiva::cast(leftExpr, rightType), rightExpr);
      }
    }
    return expr;
  }

  else if (expr->getType() == LESS_THAN) {
    auto ltExpr = std::static_pointer_cast<fpdb::expression::gandiva::LessThan>(expr);
    auto leftExpr = ltExpr->getLeft();
    auto rightExpr = ltExpr->getRight();
    auto leftType = getNumericType(leftExpr);
    if (leftType) {
      return lt(leftExpr, fpdb::expression::gandiva::cast(rightExpr, leftType));
    } else {
      auto rightType = getNumericType(rightExpr);
      if (rightType) {
        return lt(fpdb::expression::gandiva::cast(leftExpr, rightType), rightExpr);
      }
    }
    return expr;
  }

  else if (expr->getType() == LESS_THAN_OR_EQUAL_TO) {
    auto leExpr = std::static_pointer_cast<fpdb::expression::gandiva::LessThanOrEqualTo>(expr);
    auto leftExpr = leExpr->getLeft();
    auto rightExpr = leExpr->getRight();
    auto leftType = getNumericType(leftExpr);
    if (leftType) {
      return lte(leftExpr, fpdb::expression::gandiva::cast(rightExpr, leftType));
    } else {
      auto rightType = getNumericType(rightExpr);
      if (rightType) {
        return lte(fpdb::expression::gandiva::cast(leftExpr, rightType), rightExpr);
      }
    }
    return expr;
  }

  return expr;
}
