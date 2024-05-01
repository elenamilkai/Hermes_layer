#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_RESULTGENERATORKERNEL_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_RESULTGENERATORKERNEL_H

#include "fpdb/catalogue/Table.h"
#include "fpdb/delta/DeltaCacheData.h"
#include <fpdb/tuple/TupleSet.h>
#include <memory>

using namespace fpdb::delta;

namespace fpdb::executor::physical::deltamerge {
class ResultGeneratorKernel {
public:
  static std::shared_ptr<TupleSet> getFinalResult(
      std::vector<std::shared_ptr<DeltaCacheData>> deltas,
      std::vector<std::shared_ptr<TupleSet>> stables,
      std::vector<std::shared_ptr<TupleSet>> diskDeltas,
      std::vector<int> diskDeltasTimestamps,
      std::unordered_map<int, std::set<int>> deleteMap,
      std::vector<std::string> &projectColumnNames,
      std::shared_ptr<fpdb::catalogue::Table> table
      );
};
} // namespace fpdb::executor::physical::deltamerge

#endif // FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_RESULTGENERATORKERNEL_H
