//
// Created by matt on 8/7/20.
//

#include <fpdb/executor/physical/cache/CacheLoadPOp.h>
#include <fpdb/executor/physical/cache/CacheHelper.h>
#include <fpdb/executor/message/ScanMessage.h>
#include <fpdb/executor/message/TupleMessage.h>
#include <fpdb/executor/message/cache/CacheMetricsMessage.h>
#include <utility>

using namespace fpdb::executor::physical::cache;

CacheLoadPOp::CacheLoadPOp(std::string name,
           std::vector<std::string> projectColumnNames,
           int nodeId,
           std::vector<std::string> predicateColumnNames,
           std::vector<std::string> columnNames,
					 std::shared_ptr<Partition> Partition,
					 int64_t StartOffset,
					 int64_t FinishOffset,
           S3ClientType s3ClientType) :
  PhysicalOp(std::move(name), CACHE_LOAD, std::move(projectColumnNames), nodeId),
  predicateColumnNames_(std::move(predicateColumnNames)),
  columnNames_(std::move(columnNames)),
  partition_(std::move(Partition)),
  startOffset_(StartOffset),
  finishOffset_(FinishOffset),
  s3ClientType_(s3ClientType) {}

std::string CacheLoadPOp::getTypeString() const {
  return "CacheLoadPOp";
}

void CacheLoadPOp::onReceive(const Envelope &message) {
  if (message.message().type() == MessageType::START) {
	  this->onStart();
  } else if (message.message().type() == MessageType::LOAD_RESPONSE) {
    auto loadResponseMessage = dynamic_cast<const LoadResponseMessage &>(message.message());
    this->onCacheLoadResponse(loadResponseMessage);
  } else {
    ctx()->notifyError("Unrecognized message type " + message.message().getTypeString());
  }
}

void CacheLoadPOp::onStart() {
  /**
   * We always have hitOperator_ and missOperatorToCache_
   * In hybrid caching, we also have missOperatorToPushdown_; in pullup caching, we don't
   */

  if (!hitOperatorName_.has_value())
	ctx()->notifyError("Hit consumer not set ");

  if (!missOperatorToCacheName_.has_value())
	ctx()->notifyError("Miss caching consumer not set");

  if (missOperatorToPushdownName_.has_value()) {
    SPDLOG_DEBUG("Starting operator  |  name: '{}', hitOperator: '{}', missOperatorToCache: '{}', missOperatorToPushdown: '{}'",
                 this->name(),
                 *hitOperatorName_,
                 *missOperatorToCacheName_,
                 *missOperatorToPushdownName_);
  } else {
    SPDLOG_DEBUG("Starting operator  |  name: '{}', hitOperator: '{}', missOperatorToCache: '{}'",
                 this->name(),
                 *hitOperatorName_,
                 *missOperatorToCacheName_);
  }

  requestLoadSegmentsFromCache();
}

void CacheLoadPOp::requestLoadSegmentsFromCache() {
  CacheHelper::requestLoadSegmentsFromCache(columnNames_, partition_, startOffset_, finishOffset_, name(), ctx());
}

void CacheLoadPOp::onCacheLoadResponse(const LoadResponseMessage &Message) {
  std::vector<std::shared_ptr<Column>> hitColumns;
  std::vector<std::string> hitColumnNames;
  std::vector<std::string> missedCachingColumnNames;
  std::vector<std::string> missedPushdownColumnNames;

  /**
   * 1. Gather hit and missing segments
   */
  // Gather missed caching segment columns
  const auto& segmentKeysToCache = Message.getSegmentKeysToCache();
  missedCachingColumnNames.reserve(segmentKeysToCache.size());
  for (auto const &segmentKey: segmentKeysToCache) {
    missedCachingColumnNames.emplace_back(segmentKey->getColumnName());
  }

  const auto& hitSegments = Message.getSegments();
  SPDLOG_DEBUG("Loaded segments from cache  |  numRequested: {}, numHit: {}", columnNames_.size(), hitSegments.size());

  // Gather hit segment columns and missed pushdown segment columns
  for (const auto &columnName: columnNames_) {
    auto segmentKey = SegmentKey::make(partition_, columnName, SegmentRange::make(startOffset_, finishOffset_));
    auto segment = hitSegments.find(segmentKey);
    if (segment != hitSegments.end()) {
      hitColumns.emplace_back(segment->second->getColumn());
      hitColumnNames.emplace_back(columnName);
    } else if (std::find(missedCachingColumnNames.begin(), missedCachingColumnNames.end(), columnName)
               == missedCachingColumnNames.end()) {
      /**
       * A trick here: no need to check whether it's in projectedColumnNames_,
       * because if resultNeeded = true, missedPushdownColumnNames are all in projectedColumnNames_,
       * otherwise we make missedPushdownColumnNames as all projectedColumnNames_
       */
      missedPushdownColumnNames.emplace_back(columnName);
	  }
  }

  /**
   * 2. Check whether the processing on cache data is needed
   */
  bool cachingResultNeeded;

  // FIXME: Airmettle doesn't support intra-partition hybrid as it doesn't preserve order
  if (s3ClientType_ != AIRMETTLE) {
    /**
     * Caching result is not needed when:
     *    hitColumns + missCachingColumns don't cover all predicateColumns or
     *    hitColumns + missCachingColumns cover no projectedColumns
     */
    cachingResultNeeded = true;
    for (auto const &predicateColumnName: predicateColumnNames_) {
      if (std::find(hitColumnNames.begin(), hitColumnNames.end(), predicateColumnName) == hitColumnNames.end() &&
          std::find(missedCachingColumnNames.begin(), missedCachingColumnNames.end(), predicateColumnName) ==
          missedCachingColumnNames.end()) {
        cachingResultNeeded = false;
        break;
      }
    }
    if (cachingResultNeeded) {
      cachingResultNeeded = false;
      for (auto const &projectedColumnName: getProjectColumnNames()) {
        if (std::find(hitColumnNames.begin(), hitColumnNames.end(), projectedColumnName) != hitColumnNames.end() ||
            std::find(missedCachingColumnNames.begin(), missedCachingColumnNames.end(), projectedColumnName) !=
            missedCachingColumnNames.end()) {
          cachingResultNeeded = true;
          break;
        }
      }
    }
  } else {
    /**
     * Caching result is not needed when:
     *    hitColumns + missCachingColumns don't cover all (no intra-partition hybrid)
     */
    cachingResultNeeded = (hitColumnNames.size() + missedCachingColumnNames.size() == columnNames_.size());
  }

  /**
   * 3. Send different messages to corresponding consumers
   */
  // Send the hit columns to the hit operator if result needed, empty tupleSet otherwise
  std::shared_ptr<TupleSet> hitTupleSet;
  if (cachingResultNeeded) {
    hitTupleSet = TupleSet::make(hitColumns);
  } else {
    hitTupleSet = TupleSet::makeWithEmptyTable();
  }
  auto hitMessage = std::make_shared<TupleMessage>(hitTupleSet, this->name());
  ctx()->send(hitMessage, *hitOperatorName_);

  if (missOperatorToPushdownName_.has_value()) {
    // Send the missed caching column names to the miss caching operator
    auto missCachingMessage = std::make_shared<ScanMessage>(missedCachingColumnNames, this->name(), cachingResultNeeded);
    ctx()->send(missCachingMessage, *missOperatorToCacheName_);

    // Send the missed pushdown column names to the miss pushdown operator
    std::shared_ptr<ScanMessage> missPushdownMessage;
    if (cachingResultNeeded) {
      missPushdownMessage = std::make_shared<ScanMessage>(missedPushdownColumnNames, this->name(), true);
    } else {
      missPushdownMessage = std::make_shared<ScanMessage>(getProjectColumnNames(), this->name(), true);
    }
    ctx()->send(missPushdownMessage, *missOperatorToPushdownName_);
  } else {
    // Send the missed caching column names to the miss caching operator
    auto missCachingMessage = std::make_shared<ScanMessage>(missedCachingColumnNames, this->name(), true);
    ctx()->send(missCachingMessage, *missOperatorToCacheName_);
  }

  /**
   * 4. Send cache metrics to segmentCacheActor
   */
  size_t hitNum, missNum;
  size_t shardHitNum, shardMissNum;
  // if not every segment is a hit then we have to either pull up or scan this shard in s3, so it is a shard miss
  // otherwise it is a shard hit
  if (hitSegments.size() == columnNames_.size()) {
    shardHitNum = 1;
    shardMissNum = 0;
  } else {
    shardHitNum = 0;
    shardMissNum = 1;
  }
  // Hybrid
  if (missOperatorToPushdownName_.has_value()) {
    if (cachingResultNeeded) {
      hitNum = hitSegments.size();
      missNum = columnNames_.size() - hitNum;
    } else {
      hitNum = 0;
      missNum = columnNames_.size();
    }
  }
  // Caching only
  else {
    if (hitSegments.size() == columnNames_.size()) {
      hitNum = hitSegments.size();
      missNum = 0;
    } else {
      hitNum = 0;
      missNum = columnNames_.size();
    }
  }
  ctx()->send(CacheMetricsMessage::make(hitNum, missNum, shardHitNum, shardMissNum, this->name()), "SegmentCache");

  /**
   * 5. Complete
   */
  ctx()->notifyComplete();
}

void CacheLoadPOp::setHitOperator(const std::shared_ptr<PhysicalOp> &op) {
  this->hitOperatorName_ = op->name();
  this->produce(op);
}

void CacheLoadPOp::setMissOperatorToCache(const std::shared_ptr<PhysicalOp> &op) {
  this->missOperatorToCacheName_ = op->name();
  this->produce(op);
}

void CacheLoadPOp::setMissOperatorToPushdown(const std::shared_ptr<PhysicalOp> &op) {
  this->missOperatorToPushdownName_ = op->name();
  this->produce(op);
}

void CacheLoadPOp::clear() {
  // Noop
}
