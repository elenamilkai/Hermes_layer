//
// Created by Yifei Yang on 12/16/21.
//

#include <gandiva/tree_expr_builder.h>
#include <fpdb/expression/gandiva/DateExtract.h>
#include <sstream>

namespace fpdb::expression::gandiva {

DateExtract::DateExtract(const shared_ptr<Expression> &dateExpr, DateIntervalType intervalType):
  Expression(DATE_EXTRACT),
  dateExpr_(dateExpr), intervalType_(intervalType) {}

void DateExtract::compile(const shared_ptr<arrow::Schema> &schema) {
  dateExpr_->compile(schema);
  returnType_ = arrow::int64();

  string funcName = "extract" + intervalTypeToString(intervalType_);
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeFunction(funcName,
                                                                {dateExpr_->getGandivaExpression()},
                                                                returnType_);
}

string DateExtract::alias() {
  return "?column?";
}

string DateExtract::getTypeString() {
  return "DateExtract-" + intervalTypeToString(intervalType_);
}

set<string> DateExtract::involvedColumnNames() {
  return dateExpr_->involvedColumnNames();
}

shared_ptr<Expression> dateExtract(const shared_ptr<Expression> &dateExpr, DateIntervalType intervalType) {
  return make_shared<DateExtract>(dateExpr, intervalType);
}

}
