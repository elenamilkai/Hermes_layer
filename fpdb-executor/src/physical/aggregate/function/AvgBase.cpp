//
// Created by Yifei Yang on 1/26/22.
//

#include <fpdb/executor/physical/aggregate/function/AvgBase.h>
#include <arrow/compute/api_aggregate.h>
#include <arrow/compute/cast.h>

namespace fpdb::executor::physical::aggregate {

AvgBase::AvgBase(AggregateFunctionType type,
                 const string &outputColumnName,
                 const shared_ptr<fpdb::expression::gandiva::Expression> &expression) :
  AggregateFunction(type, outputColumnName, expression) {}

shared_ptr<arrow::DataType> AvgBase::returnType() const {
  return arrow::float64();
}

tl::expected<shared_ptr<arrow::Scalar>, string>
AvgBase::finalize(const vector<shared_ptr<AggregateResult>> &aggregateResults) {
  // build aggregate input arrays
  const auto expFinalizeSumInputArray = buildFinalizeInputArray(aggregateResults,
                                                                SUM_RESULT_KEY,
                                                                aggColumnDataType_);
  if (!expFinalizeSumInputArray) {
    return tl::make_unexpected(expFinalizeSumInputArray.error());
  }
  const auto expFinalizeCountInputArray = buildFinalizeInputArray(aggregateResults,
                                                                  COUNT_RESULT_KEY,
                                                                  arrow::int64());
  if (!expFinalizeCountInputArray) {
    return tl::make_unexpected(expFinalizeCountInputArray.error());
  }

  // compute final sum and count
  const auto &expFinalSumScalar = arrow::compute::Sum(expFinalizeSumInputArray.value());
  if (!expFinalSumScalar.ok()) {
    return tl::make_unexpected(expFinalSumScalar.status().message());
  }
  const auto &finalSumScalar = (*expFinalSumScalar).scalar();
  const auto &expFinalCountScalar = arrow::compute::Sum(expFinalizeCountInputArray.value());
  if (!expFinalCountScalar.ok()) {
    return tl::make_unexpected(expFinalCountScalar.status().message());
  }
  const auto &finalCountScalar = (*expFinalCountScalar).scalar();

  // compute average
  const auto &expAvgScalar = arrow::compute::CallFunction("divide", {finalSumScalar, finalCountScalar});
  if (!expAvgScalar.ok()) {
    return tl::make_unexpected(expAvgScalar.status().message());
  }
  const auto &avgScalar = (*expAvgScalar).scalar();

  // cast to float64 to avoid implicit cast at downstream
  const auto &expCastScalar = arrow::compute::Cast(avgScalar, arrow::float64());
  if (!expCastScalar.ok()) {
    return tl::make_unexpected(expCastScalar.status().message());
  }
  return (*expCastScalar).scalar();
}

}
