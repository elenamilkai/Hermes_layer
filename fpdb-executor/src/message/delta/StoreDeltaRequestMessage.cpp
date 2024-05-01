//
// Created by Elena Milkai on 10/14/21.
//

#include <fpdb/executor/message/delta/StoreDeltaRequestMessage.h>
#include <fmt/format.h>

using namespace fpdb::executor::message;

StoreDeltaRequestMessage::StoreDeltaRequestMessage(std::shared_ptr<Table> table, std::shared_ptr<DeltaCacheKey> tailKey,
                                                   std::string sender) :
                Message(STORE_REQUEST, sender),
                table_(std::move(table)),
                tailKey_(std::move(tailKey)) {}

std::shared_ptr<StoreDeltaRequestMessage> StoreDeltaRequestMessage::make(std::shared_ptr<Table> table,
                                                                         std::shared_ptr<DeltaCacheKey> tailKey,
                                                                         std::string sender) {
  return std::make_shared<StoreDeltaRequestMessage>(table, tailKey, sender);
}

std::string StoreDeltaRequestMessage::getTypeString() const {
  return "StoreDeltaRequestMessage";
}

std::shared_ptr<Table> StoreDeltaRequestMessage::getTable() const {
  return this->table_;
}

std::shared_ptr<DeltaCacheKey> StoreDeltaRequestMessage::getTailKey() const {
  return this->tailKey_;
}

std::string StoreDeltaRequestMessage::toString() const {
  return fmt::format("tailKey: {}", this->tailKey_->toString());
}
