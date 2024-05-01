//
// Created by Han Cao on 3/20/22.
//

#ifndef FPDB_DELTAMERGEFILTER_H
#define FPDB_DELTAMERGEFILTER_H

#include <unordered_map>
#include <gandiva/filter.h>
#include <arrow/record_batch.h>
#include <tl/expected.hpp>
#include <fpdb/expression/gandiva/Expression.h>
#include <gandiva/projector.h>
#include "fpdb/tuple/TupleSet.h"

namespace fpdb::executor::physical::deltamerge {
  class DeltaMergeFilter {
  public:
    static tl::expected<std::shared_ptr<fpdb::tuple::TupleSet>, std::string>
    evaluate(const std::vector<int> keepMap,
             std::shared_ptr<std::vector<uint8_t>> stableBm,
             std::shared_ptr<fpdb::tuple::TupleSet> tupleSet,
             std::vector<std::string> projectColumnNames,
             bool predicatesExist,
             std::string tableName);

    static tl::expected<std::shared_ptr<gandiva::SelectionVector>, std::string>
    generateSelectionVector(const std::vector<int> keepMap,
                            std::shared_ptr<std::vector<uint8_t>> stableBm);

    static void SetBit(uint8_t* bits, int64_t i);
    static int roundUp(int numToRound, int multiple);

  private:
    static std::shared_ptr<::gandiva::Projector> gandivaProjector_;
  };
}

#endif //FPDB_DELTAMERGEFILTER_H