//
// Created by Elena Milkai on 10/14/21.
//

#ifndef FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHE_H
#define FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHE_H

#include <fpdb/delta/DeltaCacheKey.h>
#include <fpdb/delta/DeltaCacheData.h>
#include <fpdb/catalogue/Table.h>
#include <memory>

using namespace fpdb::catalogue;

namespace fpdb::delta {

/**
 * DeltaCache holds table granularity. It will be initialized through DeltaCacheActor in the Executor object when
 * the system scans the catalogue. The reason for this is that there is no easy way to access the catalogue object
 * to get the table and schema information other than in fpdb-main.
 */
class DeltaCache {

public:
  explicit DeltaCache(std::shared_ptr<Table> table);

  static std::shared_ptr<DeltaCache> make(const std::shared_ptr<Table>& table);

  [[nodiscard]] std::shared_ptr<Table> getTable() const;
  std::unordered_multimap<std::shared_ptr<DeltaCacheKey>,
                          std::shared_ptr<DeltaCacheData>,
                          DeltaKeyPointerHash,
                          DeltaKeyPointerPredicate> getMap() const;

  /**
   * Get the current cache size.
   * @return current cache size
   */
  size_t getSize() const;

  // Cache operations

  /**
   * Store the key-data pair into the cache.
   * @param key
   * @param data
   */
  void store(std::shared_ptr<DeltaCacheKey> key, std::shared_ptr<DeltaCacheData> data);

  /**
   * Load the delta from the cache with the given key. This will load all versions of delta for the given key.
   * The caller should check the length of the return vector to determine whether there is data in the cache for given
   * key.
   * @param key
   * @return an empty vector if there is no such key in the cache; otherwise, return all versions of the delta for this
   * key.
   */
  std::vector<std::shared_ptr<DeltaCacheData>> load(std::shared_ptr<DeltaCacheKey> key);

  /**
   * Remove the key-data pair from the cache (all versions of delta will be removed). This is mainly useful for
   * background merge.
   * @param key
   */
  void remove(const std::shared_ptr<DeltaCacheKey> key);

  // Print the information in the cache
  std::string getInfo();

private:
  std::shared_ptr<Table> table_; // one cache for one table
  std::unordered_multimap<std::shared_ptr<DeltaCacheKey>,
                          std::shared_ptr<DeltaCacheData>,
                          DeltaKeyPointerHash,
                          DeltaKeyPointerPredicate> map_;

};

}

#endif //FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHE_H
