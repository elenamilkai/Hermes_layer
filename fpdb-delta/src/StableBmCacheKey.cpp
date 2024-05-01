//
// Created by Elena Milkai on 2/20/23.
//

#include <fpdb/delta/StableBmCacheKey.h>
#include <fmt/format.h>

using namespace fpdb::delta;

StableBmCacheKey::StableBmCacheKey(long partition) {
  this->partition_ = partition;
}

std::shared_ptr<StableBmCacheKey> StableBmCacheKey::make(long partition) {
  return std::make_shared<StableBmCacheKey>(partition);
}

long StableBmCacheKey::getPartitionNumber() const {
  return this->partition_;
}

size_t StableBmCacheKey::hash() {
  return std::hash<long>()(partition_);
}

bool StableBmCacheKey::operator==(const StableBmCacheKey &other) const {
  return this->partition_ == other.partition_;
}

bool StableBmCacheKey::operator!=(const StableBmCacheKey &other) const {
  return !(*this == other);
}

std::string StableBmCacheKey::toString() {
  return fmt::format("{{ partition: {} }}", std::to_string(partition_));
}