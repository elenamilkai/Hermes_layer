//
// Created by Han on 09/02/22.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_DELETEMAPGENERATORKERNEL_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_DELETEMAPGENERATORKERNEL_H

#include "fpdb/catalogue/Table.h"
#include "fpdb/delta/DeltaCacheData.h"
#include "fpdb/tuple/TupleSet.h"
#include <tl/expected.hpp>
#include <unordered_set>

using namespace fpdb::delta;

namespace fpdb::executor::physical::deltamerge {

class DeleteMapGeneratorKernel {

public:

  static bool checkIfAllRecordsWereVisited(
      std::vector<int> deltaScanIndexTracker,
      std::vector<int> stableScanIndexTracker,
      std::vector<int> deltas,
      std::vector<int> stables);

  static int compare_primary_key(std::vector<int32_t> pk1, std::vector<int32_t> pk2);

  static std::tuple<std::vector<int>, std::vector<uint8_t>>
  getDeleteMap(const std::vector<std::shared_ptr<DeltaCacheData>> deltas,
               const std::vector<std::shared_ptr<TupleSet>> stables,
               std::vector<std::shared_ptr<TupleSet>> diskDeltas,
               std::vector<int> diskDeltasTimestamps,
               const std::shared_ptr<fpdb::catalogue::Table> table);
};

} // namespace fpdb::executor::physical::deltamerge

#endif // FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_DELETEMAPGENERATORKERNEL_H
