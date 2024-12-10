//
//  Created by Han on 14/02/20.
//

#include "fpdb/catalogue/Table.h"
#include "fpdb/delta/DeltaCacheKey.h"
#include "fpdb/executor/message/CompleteMessage.h"
#include "fpdb/executor/message/MessageType.h"
#include "fpdb/executor/message/TupleMessage.h"
#include "fpdb/executor/message/delta/LoadDeltaRequestMessage.h"
#include "fpdb/executor/message/delta/LoadDiskDeltasResponseMessage.h"
#include "fpdb/executor/physical/PhysicalOp.h"
#include <fpdb/executor/physical/deltamerge/DeleteMapGeneratorKernel.h>
#include <fpdb/executor/physical/deltamerge/DeltaMergePOp.h>
#include <fpdb/executor/physical/deltamerge/ResultGeneratorKernel.h>
#include <fpdb/executor/physical/deltamerge/DeltaMergeFilter.h>
#include <fpdb/delta/DeltaCacheMetadata.h>
#include <string>
#include <chrono>
#include <utility>

using namespace fpdb::executor::physical::deltamerge;
mode deltaMergeMode = keepMapCache;
bool flag = false;
DeltaMergePOp::DeltaMergePOp(std::string name,
                             std::shared_ptr<fpdb::catalogue::Table> table,
                             std::shared_ptr<fpdb::catalogue::Partition> targetPartition,
                             const std::vector<std::string> &projectColumnNames,
                             int nodeId,
                             std::string cacheHandlerName,
                             bool predicatesExist) : PhysicalOp(std::move(name), DELTA_MERGE, projectColumnNames, nodeId) {
  targetTable_ = std::move(table);
  targetPartition_ = std::move(targetPartition);
  cacheHandlerName_ = std::move(cacheHandlerName);
  predicatesExist_ = predicatesExist;
  batchCounter_ = 0;
  chunkCounter_ = 0;
  earlyStableChunkIdx_ = -1;
  lastBatch_ = false;
  keepMapCacheMode_ = make_shared<KeepMapCacheMode>();
  evaluatedData_  = nullptr;
  tableName_ = targetTable_->getName();
  projectColumnNames_ = projectColumnNames;
  stableTuplesReceived_ =  false;
  deltasReceived_ = false;
}

std::string DeltaMergePOp::getTypeString() const { return "DeltaMergePOp"; }

/**
 * @brief Execute when receive any message
 *
 * @param msg
 */
void DeltaMergePOp::onReceive(const Envelope &msg) {
  if (msg.message().type() == MessageType::START) {
    this->onStart();
//    SPDLOG_CRITICAL("{} has started..", name());
  } else if (msg.message().type() == MessageType::TUPLE) {
    // means the producer is stable from scan operator
    auto stableTupleMessage = dynamic_cast<const TupleMessage &>(msg.message());
   /// SPDLOG_CRITICAL("[{}] received {} rows from {}", name(), stableTupleMessage.tuples()->numRows(), msg.message().sender());
    stableTuplesReceived_ =  true;
    if (!flag) {
      stables_.emplace_back(stableTupleMessage.tuples());
    } else {
      batchCounter_++;
      if (stableTuplesReceived_ and deltasReceived_) {
        stablesBatchedDeltaMerge(stableTupleMessage.tuples(), batchCounter_);
      } else {
        earlyChunkBatchCnt_.emplace_back(batchCounter_);
        stables_.emplace_back(stableTupleMessage.tuples());
  ///      SPDLOG_CRITICAL(">>>>>>>>> [{}] rows {}, stables size {}, early chunk idx: {}", name(), stableTupleMessage.tuples()->numRows(), stables_.size(), earlyStableChunkIdx_);
      }
    }
  } else if (msg.message().type() == MessageType::LOAD_RESPONSE) {
    // means the producer is in-memory deltas
    auto deltaMessage = dynamic_cast<const LoadDeltaResponseMessage &>(msg.message());
//    SPDLOG_CRITICAL("DeltaMerge ID: {}", name());
    deltas_ = deltaMessage.getDeltas();
    SPDLOG_CRITICAL("{} received memory-deltas of size: {}", name(), deltas_.size());
    if (!deltas_.empty()) {
      SPDLOG_CRITICAL("{} Number of rows: {} ---> SIZE {}", name(), deltas_[0]->getDelta()->numRows(), deltas_[0]->getDelta()->size());
      if(deltas_.size()>1){
        SPDLOG_CRITICAL("{} Number of rows: {}---> SIZE {}", name(), deltas_[1]->getDelta()->numRows(), deltas_[1]->getDelta()->size());
      }
    }

    stableBm_ = deltaMessage.getStableBm();
    deltasReceived_ = true;
    if(flag and stableTuplesReceived_ and deltasReceived_) {
        if(earlyStableChunkIdx_==-1) {
          earlyStableChunkIdx_ = 0;
        }
        else {
          earlyStableChunkIdx_++;
        }
      ////  SPDLOG_CRITICAL("----------ready to send --------> [{}] batches {}, early idx {}, real batch counter {}", name(), stables_.size(), earlyStableChunkIdx_, earlyChunkBatchCnt_[earlyStableChunkIdx_]);
        stablesBatchedDeltaMerge(stables_[earlyStableChunkIdx_], earlyChunkBatchCnt_[earlyStableChunkIdx_]);
    }

  } else if (msg.message().type() == MessageType::DISK_DELTAS) {
    // means the producer is disk-deltas from scan operators
    auto diskDeltasMessage = dynamic_cast<const LoadDiskDeltasResponseMessage &>(msg.message());
    diskDeltas_.emplace_back(diskDeltasMessage.getTuples());
    diskDeltasTimestamps_.emplace_back(diskDeltasMessage.getTimestamp());
  } else if (msg.message().type() == MessageType::COMPLETE) {
    auto completeMessage = dynamic_cast<const CompleteMessage &>(msg.message());
    if(!flag) {
      // check if all producers complete sending the tuple, then we can start.
      if (allProducersComplete()) {
        deltaMerge();
      }
      this->onComplete(completeMessage);
    }
    else{

      if (allProducersComplete()) {
        while(earlyStableChunkIdx_<stables_.size()-1){
          earlyStableChunkIdx_++;
         ///// SPDLOG_CRITICAL("----------[after] ready to send --------> [{}] batches {}, early idx {}", name(), stables_.size(), earlyStableChunkIdx_);
          stablesBatchedDeltaMerge(stables_[earlyStableChunkIdx_], earlyChunkBatchCnt_[earlyStableChunkIdx_]);
        }
        deltasDeltaMerge();
        this->onComplete(completeMessage);
      }
    }
  } else {
    ctx()->notifyError("Unrecognized message type " + msg.message().getTypeString());
  }
}

/**
 * Run when DeltaMerge Starts
 */
void DeltaMergePOp::onStart() {
  auto deltaCacheKey = fpdb::delta::DeltaCacheKey::make(targetPartition_->toRawPartition());
  auto requestMessage = fpdb::executor::message::LoadDeltaRequestMessage::make(targetTable_, deltaCacheKey, name(), queryId_);
  ctx()->send(requestMessage, cacheHandlerName_);
}

/**
 * Send message to inform context that this actor is complete
 */
void DeltaMergePOp::onComplete(const CompleteMessage &) {
  if (!ctx()->isComplete() && ctx()->operatorMap().allComplete(POpRelationshipType::Producer)) {
    ctx()->notifyComplete();
  }
}

/**
 * @brief Actually start the delta merge procedure
 *
 */
void DeltaMergePOp::deltaMerge() {
  if (deltas_.empty()) {
    noMerge();
    std::shared_ptr<Message>
        tupleMessage = std::make_shared<TupleMessage>(nullptr, name());
    ctx()->tell(tupleMessage);
    return;
  }
  if(deltaMergeMode == keepMapCache){
    auto keepMapVectors  = keepMapCacheMode_->getKeepMap(deltas_,
                                                          stables_,
                                                          diskDeltas_,
                                                          diskDeltasTimestamps_,
                                                          targetTable_,
                                                          stableBm_);
    std::shared_ptr<TupleSet> evaluatedStables;
    std::shared_ptr<TupleSet> evaluatedDeltas;
    int iterStables = 0;
    int iterDeltas = 0;
    std::vector<std::shared_ptr<TupleSet>> filteredDeltas;
    auto project_stable = stables_[0]->projectExist(projectColumnNames_);
    std::vector<int> keepMap;
    auto tuple = DeltaMergeFilter::evaluate(keepMap, get<1>(keepMapVectors),
                                            project_stable.value(),
                                            projectColumnNames_,
                                            predicatesExist_,
                                            targetTable_->getName());
    evaluatedStables = tuple.value()->projectExist(projectColumnNames_).value();
    while(true) {
      iterStables++;
      if (evaluatedStables->numRows() >= iterStables*DefaultChunkSize) {
        auto buf = evaluatedStables->table()->Slice((iterStables-1)*DefaultChunkSize, DefaultChunkSize);
        /////SPDLOG_CRITICAL("[{}] stable buf {} rows.", name(), buf->num_rows());
        addToQueueAndNotify(buf);
      }
      else{
        break;
      }
    }

    // combine deltas in one TupleSet and filter deltas once
    std::shared_ptr<TupleSet> combinedMemoryDelta;
    combinedMemoryDelta = TupleSet::concatenate({deltas_[0]->getDelta()}).value(); // tail-delta in idx=0
    if(deltas_.size()>1) {
      combinedMemoryDelta = TupleSet::concatenate({combinedMemoryDelta, deltas_[deltas_.size()-1]->getDelta()}).value();
    }

    tuple = DeltaMergeFilter::evaluate(get<0>(keepMapVectors),
                                            nullptr,
                                            combinedMemoryDelta,
                                            projectColumnNames_,
                                            predicatesExist_,
                                            targetTable_->getName());

    evaluatedDeltas = TupleSet::concatenate({make_shared<TupleSet>(evaluatedStables->table()->Slice((iterStables-1)*DefaultChunkSize)),
                                                      tuple.value()->projectExist(projectColumnNames_).value()}).value();
    while(true) {
      iterDeltas++;
      if (evaluatedDeltas->numRows() >= iterDeltas*DefaultChunkSize) {
        auto buf = evaluatedDeltas->table()->Slice((iterDeltas-1)*DefaultChunkSize, DefaultChunkSize);
        /////SPDLOG_CRITICAL("[{}] stable & deltas buf {} rows.", name(), buf->num_rows());
        addToQueueAndNotify(buf);
      }
      else{
        auto buf = evaluatedDeltas->table()->Slice((iterDeltas-1)*DefaultChunkSize);
        if(buf->num_rows()!=0) {
          addToQueueAndNotify(buf);
          //////SPDLOG_CRITICAL("[{}] stable & deltas buf {} rows.", name(), buf->num_rows());
        }
        break;
      }
    }
    // send message back to cache handler.
    if(!flag) {
      ctx()->send(MergedDeltaMessage::make(this->targetTable_,
                                           this->targetPartition_->toRawPartition(),
                                           tuple.value(),
                                           get<1>(keepMapVectors),
                                           name()),
                                           "DeltaCache");
    }
    else{
      // no need to send the stable keepMap to cache, is a shared pointer, and it is updated by the
      ctx()->send(MergedDeltaMessage::make(this->targetTable_,
                                           this->targetPartition_->toRawPartition(),
                                           tuple.value(),
                                           nullptr,
                                           name()),
                                           "DeltaCache");
    }
    ////SPDLOG_CRITICAL("[{}] Final result after merge: {} stable rows, {} deltas rows", name(), evaluatedStables->numRows(), evaluatedDeltas->numRows());
    std::shared_ptr<TupleSet> combinedTupleSet = nullptr; // we do not send data to the next operators
    std::shared_ptr<Message>
        tupleMessage = std::make_shared<TupleMessage>(combinedTupleSet, name());
    ctx()->tell(tupleMessage);
  }
  else if(deltaMergeMode == resultGenerator){
    std::unordered_map<int, std::set<int>> deleteMap;
    auto finalResult = ResultGeneratorKernel::getFinalResult(deltas_,
                                                             stables_,
                                                             diskDeltas_,
                                                             diskDeltasTimestamps_,
                                                             deleteMap,
                                                             projectColumnNames_,
                                                             targetTable_);
    SPDLOG_CRITICAL(fmt::format("Table: {} Number of rows in finalResult: {}",
                                targetTable_->getName(),
                                finalResult->numRows()));
    std::shared_ptr<Message>
            tupleMessage = std::make_shared<TupleMessage>(finalResult, name());
    ctx()->tell(tupleMessage);
  }
}

void DeltaMergePOp::stablesBatchedDeltaMerge(shared_ptr<TupleSet> batch, int batchCnt) {
///  SPDLOG_CRITICAL("---------------{} is ready to start", name());
  if (deltas_.empty()) {
    addToQueueAndNotify(batch->projectExist(projectColumnNames_).value()->table());
    std::shared_ptr<Message>
        tupleMessage = std::make_shared<TupleMessage>(nullptr, name());
    ctx()->tell(tupleMessage);
    return;
  }
  /////batchCounter_++;
//  SPDLOG_CRITICAL("[{}] tuples batch size: {}, batch cnt: {}", name(), batch->numRows(), batchCnt);
  auto startTime = chrono::steady_clock::now();
  auto keepMapVectors  = keepMapCacheMode_->getBatchedKeepMap(deltas_,
                                                     batch,
                                                     diskDeltas_,
                                                     diskDeltasTimestamps_,
                                                     targetTable_,
                                                     stableBm_, batchCnt, lastBatch_);

  auto project_stable = batch->projectExist(projectColumnNames_);
  std::vector<int> keepMap;
  int batchBegin = DefaultChunkSize*(batchCnt-1);
  int batchEnd;
  if(stableBm_.get()->size()<DefaultChunkSize*batchCnt){
    batchEnd = stableBm_.get()->size();
  }
  else{
    batchEnd = DefaultChunkSize*batchCnt;
  }

  std::vector<uint8_t> subStableBM( stableBm_->begin()+batchBegin, stableBm_->begin()+batchEnd);
  auto startTime2 = chrono::steady_clock::now();
  auto tuple = DeltaMergeFilter::evaluate(keepMap, make_shared<vector<uint8_t>>(subStableBM),
                                          project_stable.value(),
                                          projectColumnNames_,
                                          predicatesExist_,
                                          name());

  /////SPDLOG_CRITICAL("[{}] batch: {} stable tuple size: {}", name(), batchCnt, tuple.value()->numRows());

  if(evaluatedData_!=nullptr){
    evaluatedData_ = TupleSet::concatenate({evaluatedData_, tuple.value()->projectExist(projectColumnNames_).value()}).value();
  }
  else {
    evaluatedData_ = tuple.value()->projectExist(projectColumnNames_).value();
  }

  if (evaluatedData_->numRows() >= (chunkCounter_+1)*DefaultChunkSize) {
    auto buf = evaluatedData_->table()->Slice(chunkCounter_*DefaultChunkSize, DefaultChunkSize);
    //////SPDLOG_CRITICAL("[{}] stable buf {} rows. Batch:  {}, Chunk: {}, Evaluated data: {} rows", name(), buf->num_rows(), batchCnt, chunkCounter_, evaluatedData_->numRows());
    addToQueueAndNotify(buf);
    chunkCounter_++;
  }

  std::shared_ptr<TupleSet> combinedTupleSet = nullptr; // we do not send data to the next operators
  std::shared_ptr<Message>
      tupleMessage = std::make_shared<TupleMessage>(combinedTupleSet, name());
  ctx()->tell(tupleMessage);
}

void DeltaMergePOp::deltasDeltaMerge(){
  if(deltas_.empty()){
    return;
  }
  //////batchCounter_++;
  lastBatch_ = true;
  auto keepMapVectors  = keepMapCacheMode_->getBatchedKeepMap(deltas_,
                                                             nullptr,
                                                             diskDeltas_,
                                                             diskDeltasTimestamps_,
                                                             targetTable_,
                                                             stableBm_, batchCounter_,
                                                             lastBatch_);

  // combine deltas in one TupleSet and filter deltas once
  shared_ptr<TupleSet> mergedDeltas;
  std::shared_ptr<TupleSet> combinedMemoryDelta;
  if (deltas_.size() > 1) {
    combinedMemoryDelta = deltas_[deltas_.size() - 1]->getDelta();
    auto tuple = DeltaMergeFilter::evaluate(get<0>(keepMapVectors),
                                            nullptr,
                                            combinedMemoryDelta,
                                            projectColumnNames_,
                                            predicatesExist_,
                                            name());

    /////SPDLOG_CRITICAL("[{}] deltas tuple size: {}", name(), tuple.value()->numRows());


    mergedDeltas = tuple.value();
    if(evaluatedData_== nullptr){
      evaluatedData_ = tuple.value()->projectExist(projectColumnNames_).value();
    }
    else {
      evaluatedData_ = TupleSet::concatenate({evaluatedData_, tuple.value()->projectExist(projectColumnNames_).value()}).value();
    }

  }
  evaluatedData_ = TupleSet::concatenate({evaluatedData_, deltas_[0]->getDelta()}).value();
  if(mergedDeltas!=nullptr){
    mergedDeltas->append(deltas_[0]->getDelta());
  } else{
    mergedDeltas = deltas_[0]->getDelta();
  }
  while(true) {
    if (evaluatedData_->numRows() >= (chunkCounter_ + 1) * DefaultChunkSize) {
      auto buf = evaluatedData_->table()->Slice(chunkCounter_ * DefaultChunkSize, DefaultChunkSize);
      /////SPDLOG_CRITICAL("[{}] stable & deltas buf {} rows. Chunk: {}, Evaluated data: {} rows", name(), buf->num_rows(),  chunkCounter_, evaluatedData_->numRows());
      addToQueueAndNotify(buf);
      chunkCounter_++;
    } else {
      auto buf = evaluatedData_->table()->Slice(chunkCounter_ * DefaultChunkSize);
      if (buf->num_rows() != 0) {
        //////SPDLOG_CRITICAL("[{}] stable & deltas buf {} rows. Chunk: {}, Evaluated data: {} rows", name(), buf->num_rows(), chunkCounter_, evaluatedData_->numRows());
        addToQueueAndNotify(buf);
      }
      break;
    }
  }
 /// SPDLOG_CRITICAL("[{}] Evaluated data: {} rows", name(), evaluatedData_->numRows());
  mergedDeltas->combine();
  ctx()->send(MergedDeltaMessage::make(this->targetTable_,
                                       this->targetPartition_->toRawPartition(),
                                       mergedDeltas,
                                       nullptr,
                                       name()),
                                       "DeltaCache");
}


void DeltaMergePOp::noMerge(){
//  SPDLOG_CRITICAL(projectColumnNames_.size());
  int iter = 0;
  while (true) {
    iter++;
    if (stables_[0]->numRows() > iter * DefaultChunkSize) {
      auto buf = stables_[0]->projectExist(projectColumnNames_).value()->table()->Slice((iter - 1) * DefaultChunkSize, DefaultChunkSize);
      addToQueueAndNotify(buf);
    } else {
      auto buf = stables_[0]->projectExist(projectColumnNames_).value()->table()->Slice((iter - 1) * DefaultChunkSize);
      if (buf->num_rows() != 0) {
        addToQueueAndNotify(buf);
      }
      break;
    }
  }

}



/**
 * Check if all the producer operators finished transferring data
 * @return: true if all producers were finished, vice versa
 */
bool DeltaMergePOp::allProducersComplete() {
  return ctx()->operatorMap().allComplete(POpRelationshipType::Producer);
}

/**
 * @brief release memory when done
 * 
 */
void DeltaMergePOp::clear() {
  stables_.clear();
  deltas_.clear();
}

void DeltaMergePOp::addToQueueAndNotify(shared_ptr<arrow::Table> batch){
  if(tableName_=="lineorder"){
    std::unique_lock<std::mutex> lock(lo_mtx_);
    lo_batchQueue.push(batch);
    lo_vBatchReady.notify_one();
    ///SPDLOG_CRITICAL("[{}]-----------------------------Ready-Lineorder {}", name(), batch->num_rows());
  } else if(tableName_=="customer"){
 //   SPDLOG_CRITICAL("-----------------------------Ready-Customer {} rows", batch->num_rows());
    std::unique_lock<std::mutex> lock(c_mtx_);
    c_batchQueue.push(batch);
    c_vBatchReady.notify_one();
  } else if(tableName_=="supplier"){
  //  SPDLOG_CRITICAL("-----------------------------Ready-Supplier {} rows", batch->num_rows());
    std::unique_lock<std::mutex> lock(s_mtx_);
    s_batchQueue.push(batch);
    s_vBatchReady.notify_one();
  } else if(tableName_=="part"){
  //  SPDLOG_CRITICAL("-----------------------------Ready-Part {} rows", batch->num_rows());
    std::unique_lock<std::mutex> lock(p_mtx_);
    p_batchQueue.push(batch);
    p_vBatchReady.notify_one();
  } else {
  //  SPDLOG_CRITICAL("-----------------------------Ready-Date {} rows", batch->num_rows());
    std::unique_lock<std::mutex> lock(d_mtx_);
    d_batchQueue.push(batch);
    d_vBatchReady.notify_one();
  }
}
