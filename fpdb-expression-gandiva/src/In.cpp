//
// Created by Yifei Yang on 12/10/21.
//

#include <fpdb/expression/gandiva/In.h>
#include <gandiva/tree_expr_builder.h>
#include <fmt/format.h>
#include <sstream>

namespace fpdb::expression::gandiva {

// makeGandivaExpression()
template<>
void In<arrow::Int32Type, int32_t>::makeGandivaExpression() {
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeInExpressionInt32(left_->getGandivaExpression(), values_);
}

template<>
void In<arrow::Int64Type, int64_t>::makeGandivaExpression() {
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeInExpressionInt64(left_->getGandivaExpression(), values_);
}

template<>
void In<arrow::DoubleType, double>::makeGandivaExpression() {
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeInExpressionDouble(left_->getGandivaExpression(), values_);
}

template<>
void In<arrow::StringType, string>::makeGandivaExpression() {
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeInExpressionString(left_->getGandivaExpression(), values_);
}

template<>
void In<arrow::Date64Type, int64_t>::makeGandivaExpression() {
  gandivaExpression_ = ::gandiva::TreeExprBuilder::MakeInExpressionDate64(left_->getGandivaExpression(), values_);
}


// alias()
template<>
string In<arrow::Int32Type, int32_t>::alias() {
  const auto &leftAlias = fmt::format("cast({} as int)",left_->alias());
  stringstream ss;
  ss << "(";
  uint i = 0;
  for (const auto &value: values_) {
    ss << value;
    if (i < values_.size() - 1) {
      ss << ", ";
    }
    ++i;
  }
  ss << ")";
  return leftAlias + " in " + ss.str();
}

template<>
string In<arrow::Int64Type, int64_t>::alias() {
  const auto &leftAlias = fmt::format("cast({} as int)",left_->alias());
  stringstream ss;
  ss << "(";
  uint i = 0;
  for (const auto &value: values_) {
    ss << value;
    if (i < values_.size() - 1) {
      ss << ", ";
    }
    ++i;
  }
  ss << ")";
  return leftAlias + " in " + ss.str();
}

template<>
string In<arrow::DoubleType, double>::alias() {
  const auto &leftAlias = fmt::format("cast({} as float)",left_->alias());
  stringstream ss;
  ss << "(";
  uint i = 0;
  for (const auto &value: values_) {
    ss << value;
    if (i < values_.size() - 1) {
      ss << ", ";
    }
    ++i;
  }
  ss << ")";
  return leftAlias + " in " + ss.str();
}

template<>
string In<arrow::StringType, string>::alias() {
  stringstream ss;
  ss << "(";
  uint i = 0;
  for (const auto &value: values_) {
    ss << "\'" + value + "\'";
    if (i < values_.size() - 1) {
      ss << ", ";
    }
    ++i;
  }
  ss << ")";
  return left_->alias() + " in " + ss.str();
}

template<>
string In<arrow::Date64Type, int64_t>::alias() {
  return "?column?";
}


// getTypeString()
template<>
string In<arrow::Int32Type, int32_t>::getTypeString() {
  return "In<Int32>";
}

template<>
string In<arrow::Int64Type, int64_t>::getTypeString() {
  return "In<Int64>";
}

template<>
string In<arrow::DoubleType, double>::getTypeString() {
  return "In<Double>";
}

template<>
string In<arrow::StringType, string>::getTypeString() {
  return "In<String>";
}

template<>
string In<arrow::Date64Type, int64_t>::getTypeString() {
  return "In<Date64>";
}

}
