//
// Created by Elena Milkai on 8/2/22.
//

#include <fpdb/delta/KeepMapCacheKey.h>
#include <fmt/format.h>

using namespace fpdb::delta;

KeepMapCacheKey::KeepMapCacheKey(long partition) {
  this->partition_ = partition;
}

std::shared_ptr<KeepMapCacheKey> KeepMapCacheKey::make(long partition) {
  return std::make_shared<KeepMapCacheKey>(partition);
}

long KeepMapCacheKey::getPartitionNumber() const {
  return this->partition_;
}

size_t KeepMapCacheKey::hash() {
  return std::hash<long>()(partition_);
}

bool KeepMapCacheKey::operator==(const KeepMapCacheKey &other) const {
  return this->partition_ == other.partition_;
}

bool KeepMapCacheKey::operator!=(const KeepMapCacheKey &other) const {
  return !(*this == other);
}

std::string KeepMapCacheKey::toString() {
  return fmt::format("{{ partition: {} }}", std::to_string(partition_));
}