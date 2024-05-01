//
// Created by Elena Milkai on 10/14/21.
//

#include <fpdb/executor/message/delta/LoadDeltaResponseMessage.h>

using namespace fpdb::executor::message;

LoadDeltaResponseMessage::LoadDeltaResponseMessage(std::shared_ptr<Table> table,
                                                   std::vector<std::shared_ptr<DeltaCacheData>> deltas,
                                                   std::shared_ptr<std::vector<uint8_t>> stableBm,
                                                   std::string sender) :
                                                   Message(LOAD_RESPONSE, sender),
                                                   table_(std::move(table)),
                                                   deltas_(std::move(deltas)),
                                                   stableBm_(std::move(stableBm)) {}

std::shared_ptr<LoadDeltaResponseMessage> LoadDeltaResponseMessage::make(std::shared_ptr<Table> table,
                                                                         std::vector<std::shared_ptr<DeltaCacheData>> deltas,
                                                                         std::shared_ptr<std::vector<uint8_t>> stableBm,
                                                                         std::string sender) {
  return std::make_shared<LoadDeltaResponseMessage>(std::move(table), std::move(deltas), std::move(stableBm), sender);
}

std::string LoadDeltaResponseMessage::getTypeString() const {
  return "LoadDeltaResponseMessage";
}

std::shared_ptr<Table> LoadDeltaResponseMessage::getTable() const {
  return table_;
}

std::vector<std::shared_ptr<DeltaCacheData>> LoadDeltaResponseMessage::getDeltas() const {
  return deltas_;
}

std::shared_ptr<std::vector<uint8_t>> LoadDeltaResponseMessage::getStableBm() const {
  return stableBm_;
}

std::string LoadDeltaResponseMessage::toString() const {
  std::string s = "Deltas : {";
  for (auto delta : this->deltas_) {
    s += fmt::format("data: {}, timestamp: {}",
                     delta->getDelta()->toString(), to_string(delta->getTimestamp()));
  }
  s += "}";
  return s;
}
