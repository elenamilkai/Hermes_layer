//
// Created by Elena Milkai on 10/14/21.
//

#include <fpdb/executor/delta/DeltaCacheActor.h>
#include <fpdb/executor/delta/DeltaPumpHelper.h>
#include <fpdb/catalogue/s3/S3Partition.h>
#include <fpdb/catalogue/s3/S3Table.h>
#include <fpdb/executor/physical/deltamerge/DeleteMapGeneratorKernel.h>
#include <fpdb/delta/DeltaCacheMetadata.h>
#include <spdlog/spdlog.h>

using namespace fpdb::executor::delta;
using namespace fpdb::catalogue::s3;
using namespace fpdb::executor::physical::deltamerge;

[[maybe_unused]] behavior DeltaCacheActor::makeBehaviour(stateful_actor<DeltaCacheActorState> *self, const std::vector<std::shared_ptr<Table>>& tables) {
  // Construct the delta cache for all the tables
  for (const auto& table : tables) {
    self->state.cache[table] = DeltaCache::make(table);
    self->state.stableBmCache[table] = StableBmCache::make(table);
  }
  self->attach_functor([=](const error &reason) {
    SPDLOG_DEBUG("[Actor {} ('<name unavailable>')]  DeltaCacheActor exit  |  reason: {}",
                 self->id(), to_string(reason));

    // reset the cache
    for (auto table : self->state.cache) {
      table.second.reset();
    }
  });
  return {
    [=](LoadDeltaAtom, const std::shared_ptr<LoadDeltaRequestMessage> &msg) {  // triggered by DeltaMerge operator
        return loadMemoryDelta(*msg, self);
    },
    [=](CacheDeltaAtom) {  // triggered by PeriodicGetTail thread
        deltaPump(self);

    },
    [=](MergedDeltaAtom, const std::shared_ptr<MergedDeltaMessage> &msg){
        storeMergedDelta(*msg, self);
      }
  };
}
std::shared_ptr<LoadDeltaResponseMessage> DeltaCacheActor::loadMemoryDelta(const LoadDeltaRequestMessage &msg,
                                                                            stateful_actor<DeltaCacheActorState> *self) {
  if(msg.getQueryId() != self->state.queryId){
    self->state.queryId = msg.getQueryId();
      deltaPump(self);
  }
  const std::shared_ptr<DeltaCacheKey> deltaCacheKey = msg.getDeltaKey();
  auto table = msg.getTable();
  if (self->state.cache.find(table) == self->state.cache.end()) {
    throw runtime_error(fmt::format("No matching table {} found", table->getName()));
  }
  std::shared_ptr<DeltaCache> cache = self->state.cache[table];
  std::shared_ptr<DeltaCacheKey> cacheKey = msg.getDeltaKey();
  std::vector<std::shared_ptr<DeltaCacheData>> requestedDeltas = cache->load(cacheKey);
  std::shared_ptr<std::vector<uint8_t>> stableBm;
  shared_ptr<LoadDeltaResponseMessage> response;
  shared_ptr<StableBmCacheData> stableBmCacheData;
  if(deltaMergeMode == keepMapCache) {
    std::shared_ptr<StableBmCache> stableBmCache = self->state.stableBmCache[table];
    if (stableBmCache->getSize() != 0) {
      auto stableBmCacheKey = StableBmCacheKey::make(msg.getDeltaKey()->getPartitionNumber());
      stableBmCacheData = stableBmCache->load(stableBmCacheKey);
      if (stableBmCacheData != nullptr) {
        stableBm = stableBmCacheData->getStableBm();
      }
    }
  }
  response = LoadDeltaResponseMessage::make(table, requestedDeltas,  stableBm, self->state.name);
  return response;
}

void DeltaCacheActor::deltaPump(stateful_actor<DeltaCacheActorState>* self) {
  // Construct all schema map from the table for DeltaPump
  std::unordered_map<std::string, std::shared_ptr<::arrow::Schema>> allSchema;
  std::unordered_map<std::string, long> partitionInfo;
  for (const auto &tableCache : self->state.cache) {
    std::shared_ptr<Table> table = tableCache.first;
    allSchema[table->getName()] = table->getDeltaSchema();
    auto targetS3Table = dynamic_pointer_cast<S3Table>(table);
    partitionInfo[table->getName()] = (long) targetS3Table->getS3Partitions().size();
  }
  /*if (self->state.lastTimestamp == 0) {// ignore the very first pump of the very beginning
    auto deltasTimestamp = DeltaPumpHelper::pump(allSchema, partitionInfo);
  }*/

  auto deltasTimestamp = DeltaPumpHelper::pump(allSchema, partitionInfo);
  auto allDeltas = deltasTimestamp.first;
  self->state.lastTimestamp = deltasTimestamp.second;
  // Load the deltas to cache
  for (const auto &tableDelta : allDeltas) {
    std::string tableName = tableDelta.first;
    std::shared_ptr<DeltaCache> targetCache;
    // Find the correct cache for the table
    for (const auto &tableCache : self->state.cache) {
      auto tableObj = tableCache.first;
      if (tableObj->getName() == tableName) {
        targetCache = tableCache.second;
        break;
      }
    }
    // construct cache key and cache data
    for (const auto &delta : tableDelta.second) {
      long partNum = delta.first;
      auto targetS3Table = dynamic_pointer_cast<S3Table>(targetCache->getTable());
      auto cacheKey = DeltaCacheKey::make(partNum);
      auto cacheData = DeltaCacheData::make(delta.second, self->state.lastTimestamp);
      targetCache->store(cacheKey, cacheData);
    }
  }

}


void DeltaCacheActor::storeMergedDelta(const MergedDeltaMessage& msg,
                                       stateful_actor<DeltaCacheActorState> *self){
  auto table = msg.getTable();
  auto deltaKey = DeltaCacheKey::make(msg.getPartition());
  auto mergedDelta =  msg.getMergedDelta();
  auto stableBm = msg.getStableBm();
  std::shared_ptr<DeltaCache> targetCache;
  for (const auto& tableCache : self->state.cache) {
    auto tableObj = tableCache.first;
    if (tableObj->getName() == table->getName()) {
      targetCache = tableCache.second;
      break;
    }
  }
  targetCache->remove(deltaKey);
  std::shared_ptr<DeltaCacheData> cacheData = DeltaCacheData::make(mergedDelta, self->state.lastTimestamp);
  targetCache->store(deltaKey, cacheData);
  if(stableBm!= nullptr){
    auto stableBmCacheKey = StableBmCacheKey::make(msg.getPartition());
    std::shared_ptr<StableBmCache> targetStableBmCache;
    for (const auto& stableBmCache : self->state.stableBmCache) {
      auto tableObj = stableBmCache.first;
      if (tableObj->getName() == table->getName()) {
        targetStableBmCache = stableBmCache.second;
        break;
      }
    }
    targetStableBmCache->remove(stableBmCacheKey);
    std::shared_ptr<StableBmCacheData> stableBmData = StableBmCacheData::make(stableBm);
    targetStableBmCache->store( stableBmCacheKey, stableBmData);
  }
}

