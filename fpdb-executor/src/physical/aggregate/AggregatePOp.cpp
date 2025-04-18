//
// Created by matt on 11/12/19.
//

#include <fpdb/executor/physical/aggregate/AggregatePOp.h>
#include <fpdb/executor/physical/aggregate/AggregateResult.h>
#include <fpdb/executor/physical/PhysicalOp.h>
#include <fpdb/executor/message/CompleteMessage.h>
#include <fpdb/executor/message/TupleMessage.h>
#include <fpdb/executor/message/Message.h>
#include <arrow/scalar.h>
#include <string>
#include <utility>
#include <memory>

namespace fpdb::executor::physical::aggregate {

AggregatePOp::AggregatePOp(string name,
                           vector<string> projectColumnNames,
                           int nodeId,
                           vector<shared_ptr<aggregate::AggregateFunction>> functions):
  PhysicalOp(move(name), AGGREGATE, move(projectColumnNames), nodeId),
  functions_(move(functions)) {

  // initialize aggregate results
  for (uint i = 0; i < functions_.size(); ++i) {
    aggregateResults_.emplace_back(vector<shared_ptr<AggregateResult>>{});
  }
}

std::string AggregatePOp::getTypeString() const {
  return "AggregatePOp";
}

void AggregatePOp::onStart() {
  SPDLOG_CRITICAL("Starting operator  |  name: '{}'", this->name());
}

void AggregatePOp::onReceive(const Envelope &message) {
  if (message.message().type() == MessageType::START) {
    this->onStart();
  } else if (message.message().type() == MessageType::TUPLE) {
    auto tupleMessage = dynamic_cast<const TupleMessage &>(message.message());
    this->onTuple(tupleMessage);
  } else if (message.message().type() == MessageType::COMPLETE) {
    auto completeMessage = dynamic_cast<const CompleteMessage &>(message.message());
    this->onComplete(completeMessage);
  } else {
    ctx()->notifyError("Unrecognized message type " + message.message().getTypeString());
  }
}

void AggregatePOp::onTuple(const TupleMessage &message) {
  //compute(message.tuples());
}

void AggregatePOp::onComplete(const CompleteMessage &) {
  if (!ctx()->isComplete() &&
      this->ctx()->operatorMap().allComplete(fpdb::executor::physical::POpRelationshipType::Producer)) {

    // Finalize
    /*shared_ptr<TupleSet> tupleSet;
    if (hasResult()) {
      tupleSet = finalize();
    } else {
      tupleSet = finalizeEmpty();
    }

    shared_ptr<Message> tupleMessage = make_shared<TupleMessage>(tupleSet, this->name());
    ctx()->tell(tupleMessage);*/
    ctx()->notifyComplete();
  }
}

void AggregatePOp::compute(const shared_ptr<TupleSet> &tupleSet) {
  // compute and save aggregate results
  for (uint i = 0; i < functions_.size(); ++i) {
    const auto &expAggregateResult = functions_[i]->computePartial(tupleSet);
    if (!expAggregateResult.has_value()) {
      ctx()->notifyError(expAggregateResult.error());
    }
    aggregateResults_[i].emplace_back(expAggregateResult.value());
  }
}

shared_ptr<TupleSet> AggregatePOp::finalize() {
  // Create output schema
  vector<shared_ptr<arrow::Field>> fields;
  for (const auto &function: functions_) {
    shared_ptr<arrow::Field> field = arrow::field(function->getOutputColumnName(), function->returnType());
    fields.emplace_back(field);
  }
  const auto &schema = arrow::schema(fields);

  // Create output tuple
  vector<shared_ptr<arrow::Array>> columns;
  for (uint i = 0; i < functions_.size(); ++i) {
    const auto &function = functions_[i];
    // Finalize
    const auto &expFinalResult = function->finalize(aggregateResults_[i]);
    if (!expFinalResult.has_value()) {
      ctx()->notifyError(expFinalResult.error());
    }

    // Make the column of the final result
    const auto &finalResult = expFinalResult.value();
    if (function->returnType() == arrow::int32()) {
      auto colArgh = makeArgh<arrow::Int32Type>(static_pointer_cast<arrow::Int32Scalar>(finalResult));
      columns.emplace_back(colArgh.value());
    } else if (function->returnType() == arrow::int64()) {
      auto colArgh = makeArgh<arrow::Int64Type>(static_pointer_cast<arrow::Int64Scalar>(finalResult));
      columns.emplace_back(colArgh.value());
    } else if (function->returnType() == arrow::float64()) {
      auto colArgh = makeArgh<arrow::DoubleType>(static_pointer_cast<arrow::DoubleScalar>(finalResult));
      columns.emplace_back(colArgh.value());
    } else {
      ctx()->notifyError("Unsupported aggregate output field type " + function->returnType()->name());
    }
  }

  // Make tupleSet
  const auto &table = arrow::Table::Make(schema, columns);
  const shared_ptr<TupleSet> &aggregatedTuples = TupleSet::make(table);

  // Project using projectColumnNames
  auto expProjectTupleSet = aggregatedTuples->projectExist(getProjectColumnNames());
  if (!expProjectTupleSet) {
    ctx()->notifyError(expProjectTupleSet.error());
  }

  SPDLOG_DEBUG("Completing  |  Aggregation result: \n{}", aggregatedTuples->toString());
  return expProjectTupleSet.value();
}

shared_ptr<TupleSet> AggregatePOp::finalizeEmpty() {
  arrow::FieldVector fields;
  for (const auto &function: functions_) {
    fields.emplace_back(make_shared<arrow::Field>(function->getOutputColumnName(), function->returnType()));
  }
  auto outputSchema = arrow::schema(fields);
  return TupleSet::make(outputSchema);
}

bool AggregatePOp::hasResult() {
  return !aggregateResults_[0].empty();
}

void AggregatePOp::clear() {
  aggregateResults_.clear();
}

}