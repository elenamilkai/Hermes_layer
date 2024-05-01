//
// Created by Elena Milkai on 5/22/22.
//

#include <fpdb/executor/message/delta/MergedDeltaMessage.h>

#include <utility>

using namespace fpdb::executor::message;

MergedDeltaMessage::MergedDeltaMessage(std::shared_ptr<Table> table,
                                       long partition,
                                       std::shared_ptr<TupleSet> mergedDeltaTuples,
                                       std::shared_ptr<std::vector<uint8_t>> stableBm,
                                       std::string sender) :
  Message(MERGED_DELTA, std::move(sender)),
  table_(std::move(table)),
  partition_(partition),
  mergedDeltaTuples_(std::move(mergedDeltaTuples)) {
  stableBm_ = std::move(stableBm);
}

std::shared_ptr<MergedDeltaMessage> MergedDeltaMessage::make(std::shared_ptr<Table> table,
                                                             long partition,
                                                             std::shared_ptr<TupleSet> mergedDeltaTuples,
                                                             std::shared_ptr<std::vector<uint8_t>> stableBm,
                                                             const std::string& sender) {
  return std::make_shared<MergedDeltaMessage>(std::move(table),
                                              partition,
                                              std::move(mergedDeltaTuples),
                                              std::move(stableBm),
                                              sender);
}

std::string MergedDeltaMessage::getTypeString() const {
  return "MergedDeltaMessage";
}

std::shared_ptr<Table> MergedDeltaMessage::getTable() const {
  return table_;
}

long MergedDeltaMessage::getPartition() const {
  return partition_;
}

std::shared_ptr<TupleSet> MergedDeltaMessage::getMergedDelta() const {
  return mergedDeltaTuples_;
}


std::shared_ptr<std::vector<uint8_t>> MergedDeltaMessage::getStableBm() const {
  return stableBm_;
}

std::string MergedDeltaMessage::toString() const {
  std::string s =
      fmt::format("Merged-Delta: {}, table: {}, deltaKey: {}",
                  this->getMergedDelta()->toString(),
                  this->table_->getName(),
                  to_string(this->partition_));
  return s;
}

