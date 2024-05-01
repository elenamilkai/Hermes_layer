//
// Created by Elena Milkai on 2/20/23.
//
#include <fpdb/delta/StableBmCache.h>
#include <utility>

using namespace fpdb::delta;

StableBmCache::StableBmCache(std::shared_ptr<Table> table) {
  this->table_ = std::move(table);
}

std::shared_ptr<StableBmCache> StableBmCache::make(const std::shared_ptr<Table>& table) {
  return std::make_shared<StableBmCache>(table);
}

std::shared_ptr<Table> StableBmCache::getTable() const {
  return this->table_;
}

std::unordered_map<std::shared_ptr<StableBmCacheKey>,
                   std::shared_ptr<StableBmCacheData>,
                   StableBmKeyPointerHash,
                   StableBmKeyPointerPredicate> StableBmCache::getMap() const {
  return this->map_;
}

size_t StableBmCache::getSize() const {
  return this->map_.size();
}

void StableBmCache::store(std::shared_ptr<StableBmCacheKey> key, std::shared_ptr<StableBmCacheData> data) {
  this->map_.emplace(key, data);
}

std::shared_ptr<StableBmCacheData> StableBmCache::load(std::shared_ptr<StableBmCacheKey> key) {
  // Given the key (the partition), return the stableBm
  std::shared_ptr<StableBmCacheData> stableBm;
  if(this->map_.find(key) == map_.end()){
    return nullptr;
  } else {
    stableBm = this->map_.find(key)->second;
    return stableBm;
  }
}

void StableBmCache::remove(const std::shared_ptr<StableBmCacheKey> key) {
  this->map_.erase(key);
}

std::string StableBmCache::getInfo() {
  std::string info = fmt::format("==== StableBm Cache Info [{}] ==== \n", table_->getName());
  for (auto pair : this->map_) {
    auto key = pair.first;
    auto stableBm = pair.second;
    auto partitionInfo = fmt::format("Partition: {} \n", key->toString());
    auto stableBmSize = fmt::format("Size of the keepMap: {} \n", stableBm->getStableBm()->size());
    info += partitionInfo + stableBmSize;
  }
  return info;
}