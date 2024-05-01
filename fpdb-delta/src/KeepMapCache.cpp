//
// Created by Elena Milkai on 8/2/22.
//

#include <fpdb/delta/KeepMapCache.h>
#include <utility>

using namespace fpdb::delta;

KeepMapCache::KeepMapCache(std::shared_ptr<Table> table) {
  this->table_ = std::move(table);
}

std::shared_ptr<KeepMapCache> KeepMapCache::make(const std::shared_ptr<Table>& table) {
  return std::make_shared<KeepMapCache>(table);
}

std::shared_ptr<Table> KeepMapCache::getTable() const {
  return this->table_;
}

std::unordered_map<std::shared_ptr<KeepMapCacheKey>,
                        std::shared_ptr<KeepMapCacheData>,
                        KeepMapKeyPointerHash,
                        KeepMapKeyPointerPredicate> KeepMapCache::getMap() const {
  return this->map_;
}

size_t KeepMapCache::getSize() const {
  return this->map_.size();
}

void KeepMapCache::store(std::shared_ptr<KeepMapCacheKey> key, std::shared_ptr<KeepMapCacheData> data) {
  this->map_.emplace(key, data);
}

std::shared_ptr<KeepMapCacheData> KeepMapCache::load(std::shared_ptr<KeepMapCacheKey> key) {
  // Given the key (the partition), return the keepMap
  std::shared_ptr<KeepMapCacheData> keepMap;
  if(this->map_.find(key) == map_.end()){
    return nullptr;
  } else {
    keepMap = this->map_.find(key)->second;
    return keepMap;
  }
}

void KeepMapCache::remove(const std::shared_ptr<KeepMapCacheKey> key) {
  this->map_.erase(key);
}

std::string KeepMapCache::getInfo() {
  std::string info = fmt::format("==== KeepMap Cache Info [{}] ==== \n", table_->getName());
  for (auto pair : this->map_) {
    auto key = pair.first;
    auto keepMap = pair.second;
    auto partitionInfo = fmt::format("Partition: {} \n", key->toString());
    auto keepMapSize = fmt::format("Size of the keepMap: {} \n", keepMap->getKeepMap()->size());
    info += partitionInfo + keepMapSize;
  }
  return info;
}