//
// Created by Elena Milkai on 8/2/22.
//

#ifndef FPDB_KEEPMAPCACHE_H
#define FPDB_KEEPMAPCACHE_H

#include <fpdb/delta/KeepMapCacheKey.h>
#include <fpdb/delta/KeepMapCacheData.h>
#include <fpdb/catalogue/Table.h>
#include <memory>

using namespace fpdb::catalogue;

namespace fpdb::delta {

class KeepMapCache {

public:
  explicit KeepMapCache(std::shared_ptr<Table> table);

  static std::shared_ptr<KeepMapCache> make(const std::shared_ptr<Table>& table);

  [[nodiscard]] std::shared_ptr<Table> getTable() const;
  std::unordered_map<std::shared_ptr<KeepMapCacheKey>,
                          std::shared_ptr<KeepMapCacheData>,
                          KeepMapKeyPointerHash,
                          KeepMapKeyPointerPredicate> getMap() const;

  /**
   * Get the current keepMap cache size.
   * @return current keepMap cache size
   */
  size_t getSize() const;


  /**
   * Store the key-data pair into the keepMap cache.
   * @param key
   * @param data
   */
  void store(std::shared_ptr<KeepMapCacheKey> key, std::shared_ptr<KeepMapCacheData> data);

  /**
   * Load the keepMap from the cache with the given key.
   * @param key
   * @return an empty vector if there is no such key in the cache; otherwise, return keepMap for this key
   */
  std::shared_ptr<KeepMapCacheData> load(std::shared_ptr<KeepMapCacheKey> key);

  /**
   * Remove the key-data pair from the keepMap cache
   * @param key
   */
  void remove(const std::shared_ptr<KeepMapCacheKey> key);

  // Print the information in the cache
  std::string getInfo();

private:
  std::shared_ptr<Table> table_; // one cache for one table
  std::unordered_map<std::shared_ptr<KeepMapCacheKey>,
                          std::shared_ptr<KeepMapCacheData>,
                          KeepMapKeyPointerHash,
                          KeepMapKeyPointerPredicate> map_;

};

}

#endif//FPDB_KEEPMAPCACHE_H
