//
// Created by Elena Milkai on 10/14/21.
//

#include <fpdb/delta/DeltaCacheData.h>

using namespace fpdb::delta;

DeltaCacheData::DeltaCacheData(std::shared_ptr<TupleSet> delta, long timestamp) {
  this->delta_ = delta;
  this->timestamp_ = timestamp;
}

std::shared_ptr<DeltaCacheData> DeltaCacheData::make(std::shared_ptr<TupleSet> delta, long timestamp) {
  return std::make_shared<DeltaCacheData>(delta, timestamp);
}

const std::shared_ptr<TupleSet> DeltaCacheData::getDelta() const {
  return this->delta_;
}

long DeltaCacheData::getTimestamp() const {
  return this->timestamp_;
}
