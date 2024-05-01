//
// Created by Elena Milkai on 10/14/21.
//

#include <fpdb/executor/message/delta/LoadDeltaRequestMessage.h>

using namespace fpdb::executor::message;

LoadDeltaRequestMessage::LoadDeltaRequestMessage(std::shared_ptr<Table> table, std::shared_ptr<DeltaCacheKey> deltaKey,
                                                 std::string sender, int queryId) :
    Message(LOAD_REQUEST, sender),
    table_(std::move(table)),
    deltaKey_(std::move(deltaKey)),
    queryId_(queryId){}

std::shared_ptr<LoadDeltaRequestMessage> LoadDeltaRequestMessage::make(std::shared_ptr<Table> table,
                                                                       std::shared_ptr<DeltaCacheKey> deltaKey,
                                                                       std::string sender,
                                                                       int queryId) {
  return std::make_shared<LoadDeltaRequestMessage>(std::move(table), std::move(deltaKey), sender, queryId);
}

std::string LoadDeltaRequestMessage::getTypeString() const {
  return "LoadDeltaRequestMessage";
}

std::shared_ptr<Table> LoadDeltaRequestMessage::getTable() const {
  return this->table_;
}

std::shared_ptr<DeltaCacheKey> LoadDeltaRequestMessage::getDeltaKey() const {
  return this->deltaKey_;
}

std::string LoadDeltaRequestMessage::toString() const {
  return fmt::format("deltaKey: {}", this->deltaKey_->toString());
}
int LoadDeltaRequestMessage::getQueryId() const {
  return this->queryId_;
}
