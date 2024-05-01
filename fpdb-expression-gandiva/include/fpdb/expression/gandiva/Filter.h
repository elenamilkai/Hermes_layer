//
// Created by matt on 6/5/20.
//

#ifndef FPDB_FPDB_EXPRESSION_GANDIVA_INCLUDE_FPDB_EXPRESSION_GANDIVA_FILTER_H
#define FPDB_FPDB_EXPRESSION_GANDIVA_INCLUDE_FPDB_EXPRESSION_GANDIVA_FILTER_H

#include <fpdb/expression/gandiva/Expression.h>
#include <gandiva/expression.h>
#include <gandiva/filter.h>
#include <fpdb/expression/Filter.h>
#include "Projector.h"
#include <map>

namespace fpdb::expression::gandiva {

class Filter : public fpdb::expression::Filter {

public:
  explicit Filter(std::shared_ptr<Expression> Pred);

  static std::shared_ptr<Filter> make(const std::shared_ptr<Expression> &Pred);

  static tl::expected<arrow::ArrayVector, std::string>
  evaluateBySelectionVectorStatic(const arrow::RecordBatch &recordBatch,
                                  const std::shared_ptr<::gandiva::SelectionVector> &selectionVector);

  tl::expected<std::shared_ptr<fpdb::tuple::TupleSet>, std::string>
  evaluate(const fpdb::tuple::TupleSet &TupleSet) override;

  tl::expected<std::shared_ptr<fpdb::tuple::TupleSet>, std::string>
  evaluate2(const fpdb::tuple::TupleSet &TupleSet, std::vector<std::string> projectColumnNames, std::string tableName) override;

  tl::expected<arrow::ArrayVector, std::string> evaluate(const arrow::RecordBatch &recordBatch) override;

  tl::expected<std::shared_ptr<::gandiva::SelectionVector>, std::string>
  computeSelectionVector(const arrow::RecordBatch &recordBatch);

  tl::expected<arrow::ArrayVector, std::string>
  evaluateBySelectionVector(const arrow::RecordBatch &recordBatch,
                            const std::shared_ptr<::gandiva::SelectionVector> &selectionVector);

  tl::expected<void, std::string> compile(const std::shared_ptr<fpdb::tuple::Schema> &schema) override;

private:
  std::shared_ptr<Expression> pred_;
  std::shared_ptr<::gandiva::Filter> gandivaFilter_;
  std::shared_ptr<::gandiva::Projector> gandivaProjector_;
};

}

#endif //FPDB_FPDB_EXPRESSION_GANDIVA_INCLUDE_FPDB_EXPRESSION_GANDIVA_FILTER_H
