//
// Created by Matt Woicik on 9/22/20.
//

#ifndef FPDB_FPDB_CACHE_INCLUDE_FPDB_CACHE_BELADYCACHINGPOLICY_H
#define FPDB_FPDB_CACHE_INCLUDE_FPDB_CACHE_BELADYCACHINGPOLICY_H

#include <fpdb/cache/policy/CachingPolicy.h>
#include <fpdb/cache/policy/BeladyCachingPolicyHelper.h>
#include <fpdb/cache/SegmentKey.h>
#include <memory>
#include <list>
#include <forward_list>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <set>

using namespace fpdb::cache;

namespace fpdb::cache::policy {

class BeladyCachingPolicy: public CachingPolicy {

public:
  explicit BeladyCachingPolicy(size_t maxSize,
                               std::shared_ptr<CatalogueEntry> catalogueEntry);

  void onLoad(const std::shared_ptr<SegmentKey> &key) override;
  void onRemove(const std::shared_ptr<SegmentKey> &key) override;
  std::optional<std::shared_ptr<std::vector<std::shared_ptr<SegmentKey>>>> onStore(const std::shared_ptr<SegmentKey> &key) override;
  std::shared_ptr<std::vector<std::shared_ptr<SegmentKey>>> onToCache(std::shared_ptr<std::vector<std::shared_ptr<SegmentKey>>> segmentKeys) override;
  std::shared_ptr<std::unordered_set<std::shared_ptr<SegmentKey>, SegmentKeyPointerHash, SegmentKeyPointerPredicate>> getKeysetInCachePolicy() override;
  std::string showCurrentLayout() override;

  void generateCacheDecisions(int numQueries);
  std::string approximateExecutionHitRate(int warmBatchSize, int executeBatchSize);

  [[maybe_unused]] void compareExpectedCachedKeysToActual(int queryNumberJustFinished);

  [[maybe_unused]] std::string printHitsAndMissesPerQuery();
  std::string printLayoutAfterEveryQuery();

  std::string toString() override;
  void onNewQuery() override;

private:
  std::unordered_set<std::shared_ptr<SegmentKey>, SegmentKeyPointerHash, SegmentKeyPointerPredicate> keysInCache_;
  std::unordered_map<int, std::shared_ptr<std::unordered_set<std::shared_ptr<SegmentKey>, SegmentKeyPointerHash, SegmentKeyPointerPredicate>>> queryNumToKeysInCache_;

  static bool lessKeyValue(const std::shared_ptr<SegmentKey> &key1,
                           const std::shared_ptr<SegmentKey> &key2);
  void erase(const std::shared_ptr<SegmentKey> &key);

  // Belady decisions (number of queries), this is passed in via generateCacheDecisions
  int numQueries_{};

  void assertDecreasingOrderingOfSegmentKeys(const shared_ptr<vector<shared_ptr<SegmentKey>>>& segmentKeys);
};

// helper_ to generate Belady decisions
std::shared_ptr<BeladyCachingPolicyHelper> helper_;

}

#endif //FPDB_FPDB_CACHE_INCLUDE_FPDB_CACHE_BELADYCACHINGPOLICY_H
