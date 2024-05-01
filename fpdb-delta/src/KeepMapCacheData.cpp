//
// Created by Elena Milkai on 8/2/22.
//

#include <fpdb/delta/KeepMapCacheData.h>
#include <utility>
#include <vector>

using namespace fpdb::delta;

KeepMapCacheData::KeepMapCacheData(std::shared_ptr<std::map<std::pair<int, int>, int>> keepMap) {
  this->keepMap_ = std::move(keepMap);
}

std::shared_ptr<KeepMapCacheData> KeepMapCacheData::make(const std::shared_ptr<std::map<std::pair<int, int>, int>>& keepMap) {
  return std::make_shared<KeepMapCacheData>(keepMap);
}

std::shared_ptr<std::map<std::pair<int, int>, int>> KeepMapCacheData::getKeepMap() const {
  return this->keepMap_;
}
