//
// Created by Elena Milkai on 8/2/22.
//
#ifndef FPDB_KEEPMAPCACHEMODE_H
#define FPDB_KEEPMAPCACHEMODE_H

#include "fpdb/catalogue/Table.h"
#include "fpdb/delta/DeltaCacheData.h"
#include "fpdb/tuple/TupleSet.h"
#include <tl/expected.hpp>
#include <unordered_set>

using namespace fpdb::delta;
namespace fpdb::executor::physical::deltamerge {

class KeepMapCacheMode {

public:
  KeepMapCacheMode();
  std::tuple<std::vector<int>, shared_ptr<std::vector<uint8_t>>>
  getKeepMap(const std::vector<std::shared_ptr<DeltaCacheData>> deltas,
               const std::vector<std::shared_ptr<TupleSet>> stables,
               std::vector<std::shared_ptr<TupleSet>> diskDeltas,
               std::vector<int> diskDeltasTimestamps,
               const std::shared_ptr<fpdb::catalogue::Table> table,
             shared_ptr<vector<uint8_t>> stableBm);


  std::tuple<std::vector<int>, shared_ptr<std::vector<uint8_t>>>
  getBatchedKeepMap(const std::vector<std::shared_ptr<DeltaCacheData>> deltas,
             const std::shared_ptr<TupleSet> batch,
             std::vector<std::shared_ptr<TupleSet>> diskDeltas,
             std::vector<int> diskDeltasTimestamps,
             const std::shared_ptr<fpdb::catalogue::Table> table,
             shared_ptr<vector<uint8_t>> stableBm,
             int batchCounter,
             bool lastBatch);

  int binarySearch(std::shared_ptr<Column> stablesPKColumn, int p, int r, int num);
  std::pair<int, int> positionTailAndMemDelta(const std::vector<std::shared_ptr<DeltaCacheData>> deltas);

private:
  std::vector<std::shared_ptr<Column>> tailPrimaryKeyColumns_;
  std::vector<std::shared_ptr<Column>> memPrimaryKeyColumns_;
  int tailDeltaSize_;
  std::vector<string> primaryKeyColumnNames_;
  std::shared_ptr<Column> tailTypeColumns_;
  std::map<int, int> deltasKm_;
  std::map<int, int> deltasPos_;
  std::map<std::pair<int, int>, int> deltasKm2_;
  std::map<int, std::pair<int, int>> deltasPos2_;
  std::vector<bool> seen_;
};

}
#endif//FPDB_KEEPMAPCACHEMODE_H
