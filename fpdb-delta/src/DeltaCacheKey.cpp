//
// Created by Elena Milkai on 10/14/21.
//

#include <fpdb/delta/DeltaCacheKey.h>
#include <fmt/format.h>

using namespace fpdb::delta;

DeltaCacheKey::DeltaCacheKey(long partition) {
  this->partition_ = partition;
}

std::shared_ptr<DeltaCacheKey> DeltaCacheKey::make(long partition) {
  return std::make_shared<DeltaCacheKey>(partition);
}

long DeltaCacheKey::getPartitionNumber() const {
  return this->partition_;
}

size_t DeltaCacheKey::hash() {
  return std::hash<long>()(partition_);
}

bool DeltaCacheKey::operator==(const DeltaCacheKey &other) const {
  return this->partition_ == other.partition_;
}

bool DeltaCacheKey::operator!=(const DeltaCacheKey &other) const {
  return !(*this == other);
}

std::string DeltaCacheKey::toString() {
  return fmt::format("{{ partition: {} }}", std::to_string(partition_));
}
