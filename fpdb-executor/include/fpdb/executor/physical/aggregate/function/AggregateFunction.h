//
// Created by matt on 7/3/20.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_AGGREGATE_AGGREGATEFUNCTION_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_AGGREGATE_AGGREGATEFUNCTION_H

#include <fpdb/executor/physical/aggregate/AggregateResult.h>
#include <fpdb/executor/physical/aggregate/function/AggregateFunctionType.h>
#include <fpdb/expression/Projector.h>
#include <fpdb/expression/gandiva/Expression.h>
#include <fpdb/tuple/TupleSet.h>
#include <memory>

using namespace fpdb::tuple;
using namespace std;

namespace fpdb::executor::physical::aggregate {

/**
 * Base class for aggregate functions
 */
class AggregateFunction {

public:
  explicit AggregateFunction(AggregateFunctionType type,
                             string outputColumnName,
                             shared_ptr<fpdb::expression::gandiva::Expression> expression);
  AggregateFunction() = default;
  AggregateFunction(const AggregateFunction&) = default;
  AggregateFunction& operator=(const AggregateFunction&) = default;
  virtual ~AggregateFunction() = default;

  AggregateFunctionType getType() const;
  const string &getOutputColumnName() const;
  const shared_ptr<fpdb::expression::gandiva::Expression> &getExpression() const;
  virtual shared_ptr<arrow::DataType> returnType() const;
  virtual set<string> involvedColumnNames() const;

  /**
   * Perform complete aggregation (i.e. input is complete), return an arrow::Scalar
   * @param tupleSet
   */
  virtual tl::expected<shared_ptr<arrow::Scalar>, string>
  computeComplete(const shared_ptr<TupleSet> &tupleSet) = 0;

  /**
   * Perform partial aggregation (i.e. input is partial), return an AggregateResult
   * @param tupleSet
   */
  virtual tl::expected<shared_ptr<AggregateResult>, string>
  computePartial(const shared_ptr<TupleSet> &tupleSet) = 0;

  /**
   * Finalize partial aggregate results, e.g. compute mean from a vector of sum and count values.
   * @return
   */
  virtual tl::expected<shared_ptr<arrow::Scalar>, string>
  finalize(const vector<shared_ptr<AggregateResult>> &aggregateResults) = 0;

  tl::expected<void, string> compile(const shared_ptr<arrow::Schema> &schema);

protected:
  /**
   * Used to evaluate input tupleSet using expression_
   */
  tl::expected<shared_ptr<arrow::ChunkedArray>, string> evaluateExpr(const shared_ptr<TupleSet> &tupleSet);
  void cacheInputSchema(const shared_ptr<arrow::Schema> &schema);
  void buildAndCacheProjector();

  /**
   * Build input array for finalize()
   */
  tl::expected<shared_ptr<arrow::Array>, string>
  buildFinalizeInputArray(const vector<shared_ptr<AggregateResult>> &aggregateResults,
                          const string &key,
                          const shared_ptr<arrow::DataType> &type);

  /*
   * Aggregate function type
   */
  AggregateFunctionType type_;

  /**
   * Field name of aggregate result
   */
  string outputColumnName_;

  /**
   * Compute Expression for this aggregate function, like "A*B" in sum(A * B)
   */
  shared_ptr<fpdb::expression::gandiva::Expression> expression_;

  /**
   * The schema of received tuples, sometimes cannot be known up front (e.g. when input source is a CSV file, the
   * columns aren't known until the file is read) so needs to be extracted from the first batch of tuples received
   */
  std::optional<shared_ptr<arrow::Schema>> inputSchema_;
  
  /**
   * The expression projector, created and cached when input schema is extracted from first tuple received
   */
  std::optional<shared_ptr<fpdb::expression::Projector>> projector_;

  /**
   * Data type of aggregate input column (after evaluating the expression)
   */
  shared_ptr<arrow::DataType> aggColumnDataType_;

};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_AGGREGATE_AGGREGATEFUNCTION_H
