//
// Created by Elena Milkai on 2/20/23.
//

#ifndef FPDB_STABLEBMCACHE_H
#define FPDB_STABLEBMCACHE_H
#include <fpdb/delta/StableBmCacheKey.h>
#include <fpdb/delta/StableBmCacheData.h>
#include <fpdb/catalogue/Table.h>
#include <memory>

using namespace fpdb::catalogue;

namespace fpdb::delta {

class StableBmCache {

public:
  explicit StableBmCache(std::shared_ptr<Table> table);

  static std::shared_ptr<StableBmCache> make(const std::shared_ptr<Table>& table);

  [[nodiscard]] std::shared_ptr<Table> getTable() const;
  std::unordered_map<std::shared_ptr<StableBmCacheKey>,
                     std::shared_ptr<StableBmCacheData>,
                     StableBmKeyPointerHash,
                     StableBmKeyPointerPredicate> getMap() const;

  /**
   * Get the current stableBm cache size.
   * @return current stableBm cache size
   */
  size_t getSize() const;


  /**
   * Store the key-data pair into the stableBm cache.
   * @param key
   * @param data
   */
  void store(std::shared_ptr<StableBmCacheKey> key, std::shared_ptr<StableBmCacheData> data);

  /**
   * Load the stableBm from the cache with the given key.
   * @param key
   * @return an empty vector if there is no such key in the cache; otherwise, return stableBm for this key
   */
  std::shared_ptr<StableBmCacheData> load(std::shared_ptr<StableBmCacheKey> key);

  /**
   * Remove the key-data pair from the stableBm cache
   * @param key
   */
  void remove(const std::shared_ptr<StableBmCacheKey> key);

  // Print the information in the cache
  std::string getInfo();

private:
  std::shared_ptr<Table> table_; // one cache for one table
  std::unordered_map<std::shared_ptr<StableBmCacheKey>,
                     std::shared_ptr<StableBmCacheData>,
                     StableBmKeyPointerHash,
                     StableBmKeyPointerPredicate> map_;

};

}
#endif//FPDB_STABLEBMCACHE_H
