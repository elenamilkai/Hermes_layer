//
// Created by matt on 7/3/20.
//

#include <fpdb/executor/physical/aggregate/function/AggregateFunction.h>
#include <fpdb/expression/gandiva/Projector.h>
#include <fpdb/expression/gandiva/Column.h>
#include <utility>

namespace fpdb::executor::physical::aggregate {

AggregateFunction::AggregateFunction(AggregateFunctionType type,
                                     string outputColumnName,
                                     shared_ptr<fpdb::expression::gandiva::Expression> expression) :
  type_(type),
  outputColumnName_(move(outputColumnName)),
  expression_(move(expression)) {}

AggregateFunctionType AggregateFunction::getType() const {
  return type_;
}

const string &AggregateFunction::getOutputColumnName() const {
  return outputColumnName_;
}

const shared_ptr<fpdb::expression::gandiva::Expression> &AggregateFunction::getExpression() const {
  return expression_;
}

shared_ptr<arrow::DataType> AggregateFunction::returnType() const {
  return aggColumnDataType_;
}

set<string> AggregateFunction::involvedColumnNames() const {
  if (expression_) {
    return expression_->involvedColumnNames();
  } else {
    return {};
  }
}

tl::expected<void, string> AggregateFunction::compile(const shared_ptr<arrow::Schema> &schema) {
  // if no expression then there are can be two cases:
  //   1) the function type is COUNT, which means count(*),
  //   2) the function type is AVG_REDUCE, which use pre-specified sum and count columns,
  // then we do not need to compile.
  if (!expression_) {
    if (type_ == COUNT || type_ == AVG_REDUCE) {
      return {};
    } else {
      return tl::make_unexpected("Aggregate function has no expression, it is neither COUNT nor AVG_REDUCE");
    }
  }

  // just column projection, no need to use gandiva projector
  if (expression_->getType() == expression::gandiva::COLUMN) {
    const auto &columnName = static_pointer_cast<expression::gandiva::Column>(expression_)->getColumnName();
    const auto &field = schema->GetFieldByName(columnName);
    aggColumnDataType_ = field->type();
  }

  // expression projection, need to use gandiva projector
  else {
    cacheInputSchema(schema);
    buildAndCacheProjector();
  }

  return {};
}

tl::expected<shared_ptr<arrow::ChunkedArray>, string>
AggregateFunction::evaluateExpr(const shared_ptr<TupleSet> &tupleSet) {
  // compile if not yet
  if (!aggColumnDataType_) {
    auto compileResult = compile(tupleSet->schema());
    if (!compileResult.has_value()) {
      return tl::make_unexpected(compileResult.error());
    }
  }

  // just column projection, no need to use gandiva projector
  if (expression_->getType() == expression::gandiva::COLUMN) {
    const auto &columnName = static_pointer_cast<expression::gandiva::Column>(expression_)->getColumnName();
    const auto &column = tupleSet->table()->GetColumnByName(columnName);
    return column;
  }

  // expression projection, need to use gandiva projector
  else {
    const auto &expExprTupleSet = projector_.value()->evaluate(*tupleSet);
    if (!expExprTupleSet.has_value()) {
      return tl::make_unexpected(expExprTupleSet.error());
    }
    return (*expExprTupleSet)->table()->column(0);
  }
}

void AggregateFunction::cacheInputSchema(const shared_ptr<arrow::Schema> &schema) {
  if (!inputSchema_.has_value()) {
    inputSchema_ = schema;
  }
}

void AggregateFunction::buildAndCacheProjector() {
  if (!projector_.has_value()) {
    auto expressionsVec = {this->expression_};
    projector_ = std::make_shared<expression::gandiva::Projector>(expressionsVec);
    projector_.value()->compile(inputSchema_.value());
    aggColumnDataType_ = expression_->getReturnType();
  }
}

tl::expected<shared_ptr<arrow::Array>, string>
AggregateFunction::buildFinalizeInputArray(const vector<shared_ptr<AggregateResult>> &aggregateResults,
                                           const string &key,
                                           const shared_ptr<arrow::DataType> &type) {
  // make arrayBuilder
  unique_ptr<arrow::ArrayBuilder> arrayBuilder;
  arrow::Status status = arrow::MakeBuilder(arrow::default_memory_pool(), type, &arrayBuilder);
  if (!status.ok()) {
    return tl::make_unexpected(status.message());
  }

  // append
  for (const auto &aggregateResult: aggregateResults) {
    const auto &expResultScalar = aggregateResult->get(key);
    if (!expResultScalar.has_value()) {
      return tl::make_unexpected(expResultScalar.error());
    }
    status = arrayBuilder->AppendScalar(*expResultScalar.value());
    if (!status.ok()) {
      return tl::make_unexpected(status.message());
    }
  }

  // finalize
  const auto &expArray = arrayBuilder->Finish();
  if (!expArray.ok()) {
    return tl::make_unexpected(expArray.status().message());
  }
  return expArray.ValueOrDie();
}

}