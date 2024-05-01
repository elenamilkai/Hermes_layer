//
// Created by Elena Milkai on 10/14/21.
//

#include <fpdb/delta/DeltaCache.h>
#include <utility>

using namespace fpdb::delta;

DeltaCache::DeltaCache(std::shared_ptr<Table> table) {
  this->table_ = std::move(table);
  if(this->table_->getName()=="lineorder"){
    this->map_.reserve(31);
  }
  else{
    this->map_.reserve(1);
  }
}

std::shared_ptr<DeltaCache> DeltaCache::make(const std::shared_ptr<Table>& table) {
  return std::make_shared<DeltaCache>(table);
}

std::shared_ptr<Table> DeltaCache::getTable() const {
  return this->table_;
}

std::unordered_multimap<std::shared_ptr<DeltaCacheKey>,
                        std::shared_ptr<DeltaCacheData>,
                        DeltaKeyPointerHash,
                        DeltaKeyPointerPredicate> DeltaCache::getMap() const {
  return this->map_;
}

size_t DeltaCache::getSize() const {
  size_t totalSize = 0;
  for(auto obj: this->map_){
    totalSize += obj.second->getDelta()->size();
  }
  return totalSize;
}

void DeltaCache::store(std::shared_ptr<DeltaCacheKey> key, std::shared_ptr<DeltaCacheData> data) {
  this->map_.emplace(key, data);
}

std::vector<std::shared_ptr<DeltaCacheData>> DeltaCache::load(std::shared_ptr<DeltaCacheKey> key) {
  std::vector<std::shared_ptr<DeltaCacheData>> deltas(0);
  auto results = this->map_.equal_range(key);
  for (auto it = results.first; it != results.second; ++it) {
    deltas.emplace_back(it->second);
  }
  return deltas;
}

void DeltaCache::remove(const std::shared_ptr<DeltaCacheKey> key) {
    auto erased = this->map_.erase(key);
}

std::string DeltaCache::getInfo() {
  std::string info = fmt::format("==== Cache Info [{}] ==== \n", table_->getName());
  for (auto pair : this->map_) {
    auto key = pair.first;
    auto delta = pair.second;
    auto partitionInfo = fmt::format("Partition: {} \n", key->toString());
    auto deltaSize = fmt::format("Size of the delta: {} \n", delta->getDelta()->numRows());
    info += partitionInfo + deltaSize;
  }
  return info;
}
