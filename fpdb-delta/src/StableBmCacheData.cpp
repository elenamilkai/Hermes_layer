//
// Created by Elena Milkai on 2/20/23.
//
#include <fpdb/delta/StableBmCacheData.h>
#include <utility>
#include <vector>

using namespace fpdb::delta;

StableBmCacheData::StableBmCacheData(std::shared_ptr<std::vector<uint8_t>> stableBm) {
  this->stableBm_ = std::move(stableBm);
}

std::shared_ptr<StableBmCacheData> StableBmCacheData::make(const std::shared_ptr<std::vector<uint8_t>>& stableBm) {
  return std::make_shared<StableBmCacheData>(stableBm);
}

std::shared_ptr<std::vector<uint8_t>> StableBmCacheData::getStableBm() const {
  return this->stableBm_;
}
