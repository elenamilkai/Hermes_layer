//
// Created by Elena Milkai on 8/2/22.
//

#include <fpdb/catalogue/Table.h>
#include <fpdb/executor/physical/deltamerge//KeepMapCacheMode.h>
#include <fpdb/executor/physical/deltamerge/DeleteMapGeneratorKernel.h>
#include <fpdb/delta/DeltaCacheMetadata.h>
#include "fpdb/tuple/TupleSet.h"
#include <limits>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include <unordered_set>
#include <utility>
#include <vector>
#include <chrono>
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;
using namespace fpdb::executor::physical::deltamerge;


KeepMapCacheMode::KeepMapCacheMode() {
  tailDeltaSize_ = 0;
}

int KeepMapCacheMode::binarySearch(std::shared_ptr<Column> stablesPKColumn, int p, int r, int num) {
  if (p <= r) {
    int mid = (p + r)/2;
    auto curr = stablesPKColumn->element(mid).value()->value<int32_t>().value();
    if ( curr == num)
      return mid ;
    if (curr > num)
      return binarySearch(stablesPKColumn, p, mid-1, num);
    if (curr < num)
      return binarySearch(stablesPKColumn, mid+1, r, num);
  }

  return -1;
}

std::tuple<std::vector<int>, shared_ptr<std::vector<uint8_t>>>
    KeepMapCacheMode::getKeepMap(const std::vector<std::shared_ptr<DeltaCacheData>> deltas,
                                                                           const std::vector<std::shared_ptr<TupleSet>> stables,
                                                                           std::vector<std::shared_ptr<TupleSet>> diskDeltas,
                                                                           std::vector<int> diskDeltasTimestamps,
                                                                           const std::shared_ptr<fpdb::catalogue::Table> table,
                                                                          shared_ptr<vector<uint8_t>> stableBm){
  std::tuple<std::vector<int>, shared_ptr<std::vector<uint8_t>>> keepMapVectors;
  if(!flag){  // extract the keepMap using the original algorithm
    auto keepMapVectorsKWayMerge = DeleteMapGeneratorKernel::getDeleteMap(deltas,
                                                          stables,
                                                          diskDeltas,
                                                          diskDeltasTimestamps,
                                                          table);
    keepMapVectors = std::tuple(get<0>(keepMapVectorsKWayMerge),
                                std::make_shared<std::vector<uint8_t>>(get<1>(keepMapVectorsKWayMerge)));
  } else { // extract the new keepMap with new algorithm using the cached keepMap
    int updates = 0, insertions = 0;
    long maxDeltaTimeStamp = 0;
    long minDeltaTimeStamp = 5000000000000;
    size_t pos_tail = 0;
    size_t pos_mem = 0;
    shared_ptr<TupleSet> memDelta;
    shared_ptr<TupleSet> tailDelta;
    int tailDeltaSize = 0;
    std::vector<string> primaryKeyColumnNames = table->getPrimaryKeyColumnNames();
    std::vector<std::shared_ptr<Column>> tailPrimaryKeyColumns;
    std::vector<std::shared_ptr<Column>> memPrimaryKeyColumns;
    std::vector<std::shared_ptr<Column>> stablePrimaryKeyColumns;
    std::shared_ptr<Column> tailTypeColumns;
    std::vector<int> deltasPositions;
    long stablesNumRows;
    if(deltas.size()!=1){  // when there is a new tail delta for that partition
      for (size_t i = 0; i < deltas.size(); i++) {// find the position of tail delta and the position of the memory delta
        if (deltas[i]->getTimestamp() > maxDeltaTimeStamp) {
          maxDeltaTimeStamp = deltas[i]->getTimestamp();
          pos_tail = i;
        }
        if (deltas[i]->getTimestamp() < minDeltaTimeStamp) {
          minDeltaTimeStamp = deltas[i]->getTimestamp();
          pos_mem = i;
        }
      }
      tailDelta = deltas[pos_tail]->getDelta();
      memDelta = deltas[pos_mem]->getDelta();
      tailDeltaSize = tailDelta->numRows();
    }
    else{
      pos_mem = 0;
      memDelta = deltas[pos_mem]->getDelta();
    }

    if (primaryKeyColumnNames.size() > 1) {
      std::map<std::pair<int, int>, int> deltasKm;
      std::map<int, std::pair<int, int>> deltasPos;
      for (const string &columnName : primaryKeyColumnNames) {
        memPrimaryKeyColumns.push_back(memDelta->getColumnByName(columnName).value());
        if (deltas.size() > 1) {
          tailPrimaryKeyColumns.push_back(tailDelta->getColumnByName(columnName).value());
          tailTypeColumns = tailDelta->getColumnByName("type").value();
        }
        stablePrimaryKeyColumns.push_back(stables[0]->getColumnByName(columnName).value());
      }
      stablesNumRows = stablePrimaryKeyColumns[0]->numRows();
      // create on the fly the memDelta keepMap{keys, position}
      for (int m = 0; m < memDelta->numRows(); m++) {
        auto memPkPair = std::make_pair(memPrimaryKeyColumns[0]->element(m).value()->value<int32_t>().value(),
                                        memPrimaryKeyColumns[1]->element(m).value()->value<int32_t>().value());
        deltasKm.emplace(memPkPair, m+tailDeltaSize);
        deltasPos.emplace(m+tailDeltaSize, memPkPair);
      }

      bool found = false;
      for (int j = 0; j < tailDeltaSize; j++) {
        found = false;
        auto tailPk = std::make_pair(
            tailPrimaryKeyColumns[0]->element(j).value()->value<int32_t>().value(),
            tailPrimaryKeyColumns[1]->element(j).value()->value<int32_t>().value());

        if (tailTypeColumns->element(j).value()->value<string>().value() == "UPDATE") {
          if (deltas.size() > 1) {
            auto it = deltasKm.find(tailPk);
            auto toDelete = it->second;
            if (it != deltasKm.end()) {//tail key found in memDelta
              deltasKm.erase(tailPk); // delete from the memKm the PKeys that are present in the tail-delta
              deltasKm.emplace(tailPk, j); // emplace again the PKeys with the right position in the tail-delta
              deltasPos.erase(toDelete);
              deltasPos.emplace(j, tailPk);
              found = true;
            }
          }
          if (!found and stableBm != nullptr) {
            auto pos = binarySearch(stablePrimaryKeyColumns[0], 0, stablesNumRows, tailPk.first);
            if (pos != -1) {//tail key found in stable
              (*stableBm)[pos] = 0;
              deltasKm.emplace(tailPk, j);
              deltasPos.emplace(j, tailPk);
              found = true;
            }
          }
          updates++;
        } else if (tailTypeColumns->element(j).value()->value<string>().value() == "INSERT") {
          deltasKm.emplace(tailPk, j);
          deltasPos.emplace(j, tailPk);
          insertions++;
        }
      }

      for (auto const &entry : deltasPos) {
        deltasPositions.push_back(entry.first);
      }
      shared_ptr<std::map<std::pair<int, int>, int>> stableKm;
      keepMapVectors = std::make_tuple( deltasPositions, stableBm);
    }

    else if (primaryKeyColumnNames.size()==1) {
      std::map<int, int> deltasKm;
      std::map<int, int> deltasPos;
      for (const string &columnName : primaryKeyColumnNames) {
        memPrimaryKeyColumns.push_back(memDelta->getColumnByName(columnName).value());
        if (deltas.size() > 1) {
          tailPrimaryKeyColumns.push_back(tailDelta->getColumnByName(columnName).value());
          tailTypeColumns = tailDelta->getColumnByName("type").value();
        }
        stablePrimaryKeyColumns.push_back(stables[0]->getColumnByName(columnName).value());
      }
      stablesNumRows = stablePrimaryKeyColumns[0]->numRows();
      for (int m = 0; m < memDelta->numRows(); m++) {
        auto value = m+tailDeltaSize;
        deltasKm.emplace(std::pair(memPrimaryKeyColumns[0]->element(m).value()->value<int32_t>().value(), value));
        deltasPos.emplace(std::pair(value, memPrimaryKeyColumns[0]->element(m).value()->value<int32_t>().value()));
      }
      bool found = false;
      for (int j = 0; j < tailDeltaSize; j++) {
        found = false;
        auto tailPk = tailPrimaryKeyColumns[0]->element(j).value()->value<int32_t>().value();

        if (tailTypeColumns->element(j).value()->value<string>().value() == "UPDATE") {

          if (deltas.size() > 1) {
            auto it = deltasKm.find(tailPk);
            auto toDelete = it->second;
            if (it != deltasKm.end()) {//tail key found in memDelta
              deltasKm.erase(tailPk);
              deltasKm.emplace(tailPk, j);
              deltasPos.erase(toDelete);
              deltasPos.emplace(j, tailPk);
              found = true;
            }
          }
          if (!found and stableBm != nullptr) {
            auto pos = binarySearch(stablePrimaryKeyColumns[0], 0, stablesNumRows, tailPk);
            if (pos != -1) {//tail key found in stable
              (*stableBm)[pos] = 0;
              deltasKm.emplace(tailPk, j);
              deltasPos.emplace(j, tailPk);
              found = true;
            }
          }
          updates++;
        } else if (tailTypeColumns->element(j).value()->value<string>().value() == "INSERT") {
          deltasKm.emplace(tailPk, j);
          deltasPos.emplace(j, tailPk);
          insertions++;
        }
      }
      for (auto const &entry : deltasPos) {
        deltasPositions.push_back(entry.first);
      }
      shared_ptr<std::map<std::pair<int, int>, int>> stableKm;
      keepMapVectors = std::make_tuple(deltasPositions, stableBm);
    }
  }
  return keepMapVectors;
}


std::tuple<std::vector<int>, shared_ptr<std::vector<uint8_t>>>
KeepMapCacheMode::getBatchedKeepMap(const std::vector<std::shared_ptr<DeltaCacheData>> deltas,
                             const std::shared_ptr<TupleSet> batch,
                             std::vector<std::shared_ptr<TupleSet>> diskDeltas,
                             std::vector<int> diskDeltasTimestamps,
                             const std::shared_ptr<fpdb::catalogue::Table> table,
                             shared_ptr<vector<uint8_t>> stableBm,
                             int batchCounter,
                             bool lastBatch){
  std::tuple<std::vector<int>, shared_ptr<std::vector<uint8_t>>> keepMapVectors;
  std::vector<std::shared_ptr<Column>> stablePrimaryKeyColumns;
  long stablesNumRows;
  std::vector<int> deltasPositions;

  if(batchCounter==1) {  // data extracted only in the first batch of the stables, then updated in the next batches
    shared_ptr<TupleSet> memDelta;
    shared_ptr<TupleSet> tailDelta;
    int pos_tail = -1;
    int pos_mem = -1;

    primaryKeyColumnNames_ = table->getPrimaryKeyColumnNames();
    auto pos = positionTailAndMemDelta(deltas);
    pos_mem = pos.first;
    pos_tail = pos.second;
    if (pos_tail != -1) {//there is a new tail delta
      tailDelta = deltas[pos_tail]->getDelta();
      memDelta = deltas[pos_mem]->getDelta();
      tailDeltaSize_ = tailDelta->numRows();
      seen_ = vector(tailDeltaSize_, false);
    } else {// no tail delta
      memDelta = deltas[pos_mem]->getDelta();
      seen_ = vector(0, false);
    }

    if (primaryKeyColumnNames_.size() > 1) {
      for (const string &columnName : primaryKeyColumnNames_) {
        memPrimaryKeyColumns_.push_back(memDelta->getColumnByName(columnName).value());
        if (deltas.size() > 1) {
          tailPrimaryKeyColumns_.push_back(tailDelta->getColumnByName(columnName).value());
          tailTypeColumns_ = tailDelta->getColumnByName("type").value();
        }
      }
      // create on the fly the memDelta keepMap{keys, position}
      for (int m = 0; m < memDelta->numRows(); m++) {
        auto memPkPair = std::make_pair(memPrimaryKeyColumns_[0]->element(m).value()->value<int32_t>().value(),
                                        memPrimaryKeyColumns_[1]->element(m).value()->value<int32_t>().value());
        deltasKm2_.emplace(memPkPair, m);
        deltasPos2_.emplace(m , memPkPair);
      }
    } else {
      memPrimaryKeyColumns_.push_back(memDelta->getColumnByName(primaryKeyColumnNames_[0]).value());
      if (deltas.size() > 1) {
        tailPrimaryKeyColumns_.push_back(tailDelta->getColumnByName(primaryKeyColumnNames_[0]).value());
        tailTypeColumns_ = tailDelta->getColumnByName("type").value();
      }
      // create on the fly the memDelta keepMap{keys, position}
      for (int m = 0; m < memDelta->numRows(); m++) {
        auto value = m;
        deltasKm_.emplace(std::pair(memPrimaryKeyColumns_[0]->element(m).value()->value<int32_t>().value(), value));
        deltasPos_.emplace(std::pair(value, memPrimaryKeyColumns_[0]->element(m).value()->value<int32_t>().value()));
      }
    }
  }

  if(!lastBatch) {
    if(primaryKeyColumnNames_.size() > 1){
      for (const string &columnName : primaryKeyColumnNames_) {
        stablePrimaryKeyColumns.push_back(batch->getColumnByName(columnName).value());
      }
      stablesNumRows = stablePrimaryKeyColumns[0]->numRows();
      bool found = false;
      int max = stablePrimaryKeyColumns[0]->element(stablesNumRows - 1).value()->value<int32_t>().value();
      for (int j = 0; j < tailDeltaSize_; j++) {
        if (!seen_[j]) {
          found = false;
          auto tailPk = std::make_pair(
              tailPrimaryKeyColumns_[0]->element(j).value()->value<int32_t>().value(),
              tailPrimaryKeyColumns_[1]->element(j).value()->value<int32_t>().value());
          if (tailTypeColumns_->element(j).value()->value<string>().value() == "UPDATE") {
            if (deltas.size() > 1) {
              auto startTime = chrono::steady_clock::now();
              auto it = deltasKm2_.find(tailPk);
              auto duration = duration_cast<milliseconds>(steady_clock::now() - startTime).count();
              auto toDelete = it->second;
              if (it != deltasKm2_.end()) {   //tail key found in memDelta
                deltasKm2_.erase(tailPk);     // delete from the memKm the PKeys that are present in the tail-delta
                deltasPos2_.erase(toDelete);
                found = true;
                seen_[j] = true;
              }
            }
            if (!found and stableBm != nullptr and  max >= tailPk.first) {  //
              auto pos = binarySearch(stablePrimaryKeyColumns[0], 0, stablesNumRows, tailPk.first);
              if (pos != -1) {//tail key found in stable
                (*stableBm)[(batchCounter-1)*DefaultChunkSize+pos] = 0;
                found = true;
                seen_[j] = true;
              }
            }
          }
      }
    }
  } else {
      stablePrimaryKeyColumns.push_back(batch->getColumnByName(primaryKeyColumnNames_[0]).value());
      stablesNumRows = stablePrimaryKeyColumns[0]->numRows();
      bool found = false;
      for (int j = 0; j < tailDeltaSize_; j++) {
        if (!seen_[j]) {
          found = false;
          auto tailPk = tailPrimaryKeyColumns_[0]->element(j).value()->value<int32_t>().value();
          if (tailTypeColumns_->element(j).value()->value<string>().value() == "UPDATE") {
            if (deltas.size() > 1) {
              auto it = deltasKm_.find(tailPk);
              auto toDelete = it->second;
              if (it != deltasKm_.end()) {//tail key found in memDelta
                deltasKm_.erase(tailPk);
                deltasPos_.erase(toDelete);
                found = true;
                seen_[j] = true;
              }
            }
            int max = stablePrimaryKeyColumns[0]->element(stablesNumRows - 1).value()->value<int32_t>().value();
            if (!found and stableBm != nullptr and max >= tailPk) {
              auto pos = binarySearch(stablePrimaryKeyColumns[0], 0, stablesNumRows, tailPk);
              if (pos != -1) {//tail key found in stable
                (*stableBm)[(batchCounter-1)*DefaultChunkSize+pos] = 0;
                found = true;
                seen_[j] = true;
              }
            }
          }
        }
      }
    }
  }

  if(lastBatch) {
    auto startTime = chrono::steady_clock::now();
    if(primaryKeyColumnNames_.size()>1) {
      for (auto const &entry : deltasPos2_) {
        deltasPositions.push_back(entry.first);
      }
    } else {
      for (auto const &entry : deltasPos_) {
        deltasPositions.push_back(entry.first);
      }
    }
    auto duration = duration_cast<milliseconds>(steady_clock::now() - startTime).count();
  }
  keepMapVectors = std::make_tuple(deltasPositions, stableBm);
  return keepMapVectors;
}


std::pair<int, int> KeepMapCacheMode::positionTailAndMemDelta(const std::vector<std::shared_ptr<DeltaCacheData>> deltas) {
  int pos_mem = -1;
  int pos_tail = -1;
  long maxDeltaTimeStamp = 0;
  long minDeltaTimeStamp = 5000000000000;
  if(deltas.size()!=1){  // when there is a new tail delta for that partition
    for (size_t i = 0; i < deltas.size(); i++) {// find the position of tail delta and the position of the memory delta
      if (deltas[i]->getTimestamp() > maxDeltaTimeStamp) {
        maxDeltaTimeStamp = deltas[i]->getTimestamp();
        pos_tail = i;
      }
      if (deltas[i]->getTimestamp() < minDeltaTimeStamp) {
        minDeltaTimeStamp = deltas[i]->getTimestamp();
        pos_mem = i;
      }
    }
  }
  else{
    pos_mem = 0;
  }
  return make_pair(pos_mem, pos_tail);
}
