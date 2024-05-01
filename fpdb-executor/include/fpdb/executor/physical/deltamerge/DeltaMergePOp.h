//
// Created by Han on 09/02/22.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_DELTAMERGEPOP_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_DELTAMERGEPOP_H

#include "fpdb/catalogue/Partition.h"
#include "fpdb/catalogue/Table.h"
#include "fpdb/delta/DeltaCacheData.h"
#include <fpdb/executor/physical/PhysicalOp.h>
#include <fpdb/executor/physical/deltamerge/KeepMapCacheMode.h>
#include <fpdb/tuple/TupleSet.h>
#include <memory>
#include <string>
using namespace fpdb::tuple;
namespace fpdb::executor::physical::deltamerge {
class DeltaMergePOp : public PhysicalOp {
public:
  DeltaMergePOp(std::string name, std::shared_ptr<fpdb::catalogue::Table> table,
                std::shared_ptr<fpdb::catalogue::Partition> targetPartition,
                const std::vector<std::string> &projectColumnNames,
                int nodeId, std::string cacheHandlerName,
                bool predicatesExist);

  DeltaMergePOp() = default;
  DeltaMergePOp(const DeltaMergePOp &) = default;
  DeltaMergePOp &operator=(const DeltaMergePOp &) = default;

  void onReceive(const Envelope &msg) override;
  void clear() override;
  string getTypeString() const override;


private:
  void onStart();
  void onComplete(const CompleteMessage &msg);
  bool allProducersComplete();
  void deltaMerge();
  void stablesBatchedDeltaMerge(shared_ptr<TupleSet> batch);
  void deltasDeltaMerge();
  void noMerge();
  void addToQueueAndNotify(shared_ptr<arrow::Table> batch);

  std::vector<std::shared_ptr<DeltaCacheData>> deltas_;
  std::vector<std::shared_ptr<TupleSet>> stables_;
  std::vector<std::shared_ptr<TupleSet>> diskDeltas_;
  std::vector<int> diskDeltasTimestamps_;
  std::string cacheHandlerName_;
  int batchCounter_;
  bool lastBatch_;
  int chunkCounter_;
  std::shared_ptr<TupleSet> evaluatedData_;
  std::unordered_map<int, std::unordered_set<int>> deleteMap_;
  vector<std::pair<int, int>> deletedKeys_;
  std::shared_ptr<std::vector<uint8_t>> stableBm_;
  string tableName_;

  std::shared_ptr<fpdb::catalogue::Table> targetTable_;
  std::shared_ptr<fpdb::catalogue::Partition> targetPartition_;
  bool predicatesExist_;   // true if query has predicates and false if not.
  shared_ptr<KeepMapCacheMode> keepMapCacheMode_;

  bool stableTuplesReceived_;
  bool deltasReceived_;

// caf inspect
public:
  template<class Inspector>
  friend bool inspect(Inspector& f, DeltaMergePOp& op) {
    return f.object(op).fields(f.field("name", op.name_),
                               f.field("type", op.type_),
                               f.field("projectColumnNames", op.projectColumnNames_),
                               f.field("nodeId", op.nodeId_),
                               f.field("queryId", op.queryId_),
                               f.field("opContext", op.opContext_),
                               f.field("producers", op.producers_),
                               f.field("consumers", op.consumers_),
                               f.field("deltas", op.deltas_),
                               f.field("stables", op.stables_),
                               f.field("deleteMap", op.deleteMap_),
                               f.field("targetTable", op.targetTable_),
                               f.field("deletedKeys", op.deletedKeys_),
                               f.field("targetPartition", op.targetPartition_),
                               f.field("cacheHandlerName", op.cacheHandlerName_),
                               f.field("predicatesExist", op.predicatesExist_));
  }
};

} // namespace fpdb::executor::physical::deltamerge

#endif // FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTAMERGE_DELTAMERGEPOP_H
