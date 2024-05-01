//
// Created by Elena Milkai on 3/24/22.
//

#include <fpdb/executor/physical/deltacache/CacheHandlerPOp.h>
#include <spdlog/spdlog.h>
#include <utility>

using namespace fpdb::executor::physical::deltacache;

CacheHandlerPOp::CacheHandlerPOp(std::string name,
                                 std::shared_ptr<fpdb::catalogue::Table> targetTable,
                                 std::shared_ptr<fpdb::catalogue::Partition> targetPartition,
                                 const std::vector<std::string> &projectColumnNames,
                                 int nodeId,
                                 std::string deltaMergeName):  PhysicalOp(std::move(name), CACHE_HANDLER, projectColumnNames, nodeId) {
    targetTable_ = targetTable;
    targetPartition_ = targetPartition;
    deltaMergeName_ = deltaMergeName;
}

std::shared_ptr<CacheHandlerPOp> CacheHandlerPOp::make(std::string name,
                                                       std::shared_ptr<fpdb::catalogue::Table> targetTable,
                                                       std::shared_ptr<fpdb::catalogue::Partition> targetPartition,
                                                       const std::vector<std::string> &projectColumnNames,
                                                       int nodeId,
                                                       std::string deltaMergeName) {
    return std::make_shared<CacheHandlerPOp>(name, targetTable, targetPartition, projectColumnNames, nodeId, deltaMergeName);
}

void CacheHandlerPOp::onReceive(const Envelope &msg) {
    if (msg.message().type() == MessageType::START) {
        this->onStart();
    } else if (msg.message().type() == MessageType::LOAD_REQUEST) {  //DeltaMerge sends a request to CacheHandler
        auto loadDeltasMessage = dynamic_cast<const LoadDeltaRequestMessage &>(msg.message());
        deltaMergeName_ = msg.message().sender();
        this->OnDeltasRequest(loadDeltasMessage);
    } else if (msg.message().type() == MessageType::LOAD_RESPONSE){ //DeltaCacheActor sends response to CacheHandler
        auto responseMessage = dynamic_cast<const LoadDeltaResponseMessage &>(msg.message());
        this->OnReceiveResponse(responseMessage);
    } else if(msg.message().type() == MessageType::MERGED_DELTA) {  //Received merged delta from DeltaMerge, will send it to DeltaCacheActor
        auto mergedDelta = dynamic_cast<const MergedDeltaMessage &>(msg.message());
        this->OnReceiveMergedDelta(mergedDelta);
    } else {
        throw std::runtime_error(fmt::format("Unrecognized message type: {}, {}",
                                             msg.message().type(),
                                             name()));
      }
}

void CacheHandlerPOp::onStart() {
    SPDLOG_DEBUG("Starting operator '{}'", name());
}

void CacheHandlerPOp::OnDeltasRequest(const LoadDeltaRequestMessage &message){

    const auto &table = message.getTable();
    const auto &deltaKey = message.getDeltaKey();
    const auto &sender = name();
    ctx()->send(LoadDeltaRequestMessage::make(table, deltaKey, sender, queryId_), "DeltaCache");
}


void CacheHandlerPOp::OnReceiveResponse(const LoadDeltaResponseMessage &message){
    const auto& table = message.getTable();
    const auto& deltas = message.getDeltas();
    const auto& stableBm = message.getStableBm();
    const auto &sender = name();
    std::string deltaCacheName = "DeltaMerge-"+this->targetTable_->getName()+"-"+this->targetPartition_->toString();
    ctx()->send(LoadDeltaResponseMessage::make(table, deltas, stableBm, sender), deltaMergeName_);
    ctx()->notifyComplete();
}

void CacheHandlerPOp::OnReceiveMergedDelta(const MergedDeltaMessage &message){
  SPDLOG_CRITICAL("Received merged delta");
  const auto &table = message.getTable();
  const auto &partition = message.getPartition();
  const auto& delta = message.getMergedDelta();
  auto stableBm = message.getStableBm();
  const auto &sender = name();
  ctx()->send(MergedDeltaMessage::make(table, partition, delta, stableBm, sender), "DeltaCache");
}

std::string CacheHandlerPOp::getTypeString() const { return "CacheHandlerPOp"; }

void CacheHandlerPOp::clear() {}
