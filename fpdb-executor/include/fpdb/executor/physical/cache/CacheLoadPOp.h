//
// Created by matt on 8/7/20.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_CACHE_CACHELOADPOP_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_CACHE_CACHELOADPOP_H

#include <fpdb/executor/physical/PhysicalOp.h>
#include <fpdb/executor/message/Envelope.h>
#include <fpdb/executor/message/cache/LoadResponseMessage.h>
#include <fpdb/catalogue/Partition.h>
#include <fpdb/aws/S3ClientType.h>

using namespace fpdb::executor::physical;
using namespace fpdb::executor::message;
using namespace fpdb::aws;

namespace fpdb::executor::physical::cache {

class CacheLoadPOp : public PhysicalOp {

public:
  explicit CacheLoadPOp(std::string name,
					 std::vector<std::string> projectColumnNames,
           int nodeId,
					 std::vector<std::string> predicateColumnNames,
           std::vector<std::string> columnNames,
					 std::shared_ptr<Partition> partition,
					 int64_t startOffset,
					 int64_t finishOffset,
					 S3ClientType s3ClientType);
  CacheLoadPOp() = default;
  CacheLoadPOp(const CacheLoadPOp&) = default;
  CacheLoadPOp& operator=(const CacheLoadPOp&) = default;
  ~CacheLoadPOp() override = default;

  void onReceive(const Envelope &msg) override;
  void clear() override;
  std::string getTypeString() const override;

  void setHitOperator(const std::shared_ptr<PhysicalOp> &op);
  void setMissOperatorToCache(const std::shared_ptr<PhysicalOp> &op);
  void setMissOperatorToPushdown(const std::shared_ptr<PhysicalOp> &op);

private:
  void requestLoadSegmentsFromCache();
  void onStart();
  void onCacheLoadResponse(const LoadResponseMessage &Message);

  /**
   * columnNames = projectColumnNames + predicateColumnNames
   */
  std::vector<std::string> predicateColumnNames_;
  std::vector<std::string> columnNames_;

  std::shared_ptr<Partition> partition_;
  int64_t startOffset_;
  int64_t finishOffset_;

  std::optional<std::string> hitOperatorName_;
  std::optional<std::string> missOperatorToCacheName_;
  std::optional<std::string> missOperatorToPushdownName_;

  S3ClientType s3ClientType_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, CacheLoadPOp& op) {
    return f.object(op).fields(f.field("name", op.name_),
                               f.field("type", op.type_),
                               f.field("projectColumnNames", op.projectColumnNames_),
                               f.field("nodeId", op.nodeId_),
                               f.field("queryId", op.queryId_),
                               f.field("opContext", op.opContext_),
                               f.field("producers", op.producers_),
                               f.field("consumers", op.consumers_),
                               f.field("predicateColumnNames", op.predicateColumnNames_),
                               f.field("columnNames", op.columnNames_),
                               f.field("partition", op.partition_),
                               f.field("startOffset", op.startOffset_),
                               f.field("finishOffset", op.finishOffset_),
                               f.field("hitOperator", op.hitOperatorName_),
                               f.field("missOperatorToCache", op.missOperatorToCacheName_),
                               f.field("missOperatorToPushdown", op.missOperatorToPushdownName_),
                               f.field("s3ClientType", op.s3ClientType_));
  }
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_CACHE_CACHELOADPOP_H
