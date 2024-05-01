//
// Created by Han on 18/02/22
//

#include <fpdb/catalogue/Table.h>
#include <fpdb/executor/physical/deltamerge/DeleteMapGeneratorKernel.h>
#include <limits>
#include <memory>
#include <string>
#include <tl/expected.hpp>
#include <unordered_set>
#include <vector>

using namespace fpdb::executor::physical::deltamerge;

bool DeleteMapGeneratorKernel::checkIfAllRecordsWereVisited(
    std::vector<int> deltaScanIndexTracker,
    std::vector<int> stableScanIndexTracker,
    std::vector<int> deltaSizes,
    std::vector<int> stableSizes) {
  for (size_t i = 0; i < deltaScanIndexTracker.size(); i++) {
    if (deltaScanIndexTracker[i] < deltaSizes[i]) {
      return false;
    }
  }

  for (size_t i = 0; i < stableScanIndexTracker.size(); i++) {
    if (stableScanIndexTracker[i] < stableSizes[i]) {
      return false;
    }
  }

  return true;
}

int DeleteMapGeneratorKernel::compare_primary_key(std::vector<int32_t> pk1, std::vector<int32_t> pk2) {
  if (pk1.size() != pk2.size())
    throw std::invalid_argument("The lengths of primary keys do not match");
  for (int i = 0; i < pk1.size(); i++) {
    if (pk1[i] < pk2[i]) {
      return -1;
    } else if (pk1[i] > pk2[i]) {
      return 1;
    }
  }

  return 0;
}

std::tuple<std::vector<int>, std::vector<uint8_t>>
DeleteMapGeneratorKernel::getDeleteMap(std::vector<std::shared_ptr<DeltaCacheData>> deltas,
                                       std::vector<std::shared_ptr<TupleSet>> stables,
                                       std::vector<std::shared_ptr<TupleSet>> diskDeltas,
                                       std::vector<int> diskDeltasTimestamps,
                                       std::shared_ptr<fpdb::catalogue::Table> table) {

  // initialization
  std::vector<string> primaryKeyColumnNames = table->getPrimaryKeyColumnNames();  // use table to get that
  std::vector<int> deltaScanIndexTracker;
  std::vector<int> stableScanIndexTracker;
  std::vector<std::vector<std::shared_ptr<Column>>> deltaPrimaryKeyColumns;
  std::vector<std::shared_ptr<Column>> deltaTypeColumns;
  std::vector<long> deltaTimeStampColumns;
  std::vector<std::vector<std::shared_ptr<Column>>> stablePrimaryKeyColumns;
  std::vector<int> deltaSizes;
  std::vector<int> stableSizes;
  //std::unordered_map<int, std::set<int>> deleteMap;
  std::vector<int> deltasPositions;  // vector with the keep positions of the combined deltas (memory delta and tail delta)
  std::vector<int> stablePositions; // vector with the keep positions of the stables
  std::map<std::pair<int, int>, int> stableKm;  // the stable keep map, used only in the first iteration
  std::map<int, int> stableDm; // the positions of the stables keys that will be eventually deleted since they are updated by the deltas
  std::vector<uint8_t> stableBm; // stables bit maps
  int counter = 0;

  // init necessary variables for stables
  for (size_t i = 0; i < stables.size(); i++) {
    stableSizes.push_back(stables[i]->numRows());
    std::vector<std::shared_ptr<Column>> pkColumns;
    for (const string &columnName : primaryKeyColumnNames) {
      pkColumns.push_back(stables[i]->getColumnByName(columnName).value());
    }
    stablePrimaryKeyColumns.push_back(pkColumns);
    stableScanIndexTracker.push_back(0);
  }
  // initialize the keepMap of the stables
  if(stablePrimaryKeyColumns[0].size()>1) {
    for (int s = 0; s < stables[0]->numRows(); s++) {
      stableKm.emplace(std::make_pair(stablePrimaryKeyColumns[0][0]->element(s).value()->value<int32_t>().value(),
                                      stablePrimaryKeyColumns[0][1]->element(s).value()->value<int32_t>().value()),s);
      stableBm.push_back(1);  // set all the entries of the bit map to 1, we assume we keep everything from stables
    }
  }
  else {
    for (int s = 0; s < stables[0]->numRows(); s++) {
      stableKm.emplace(std::make_pair(stablePrimaryKeyColumns[0][0]->element(s).value()->value<int32_t>().value(),0),s);
      stableBm.push_back(1);  // set all the entries of the bit map to 1, we assume we keep everything from stables
    }
  }

  // init necessary variables for deltas
  for (size_t i = 0; i < deltas.size(); i++) {
    deltaSizes.push_back(deltas[i]->getDelta()->numRows()); // size of each delta in a vector
    // the vector of pointers to each primary key column of each table
    std::vector<std::shared_ptr<Column>> pkColumns;
    for (string columnName : primaryKeyColumnNames) {
      pkColumns.push_back(deltas[i]->getDelta()->getColumnByName(columnName).value());
    }
    deltaPrimaryKeyColumns.push_back(pkColumns);
    deltaTypeColumns.push_back(deltas[i]->getDelta()->getColumnByName("type").value());
    auto cols = deltas[i]->getDelta()->table()->schema();
    deltaTimeStampColumns.push_back(deltas[i]->getTimestamp());
    deltaScanIndexTracker.push_back(0);
  }

  // Start running DeltaMerge
  while (!DeleteMapGeneratorKernel::checkIfAllRecordsWereVisited(deltaScanIndexTracker,
                                                                 stableScanIndexTracker,
                                                                 deltaSizes,
                                                                 stableSizes)) {

    // step 1: find the smallest PK among the first element of all deltas and stables
    std::vector<int32_t> currPK;
    for (int i = 0; i < primaryKeyColumnNames.size(); i++) {
      currPK.push_back(std::numeric_limits<int32_t>::max());
    }
    std::array<int, 2> minPKPosition = {0, 0};
    for (int i = 0; i < stableScanIndexTracker.size(); i++) { // loop through all stables
      if (stableScanIndexTracker[i] >= stableSizes[i])
        continue;  // skip if we already reach the bottom of the file
      std::vector<int32_t> pk;
      // add all primary key into pk
      for (int j = 0; j < primaryKeyColumnNames.size(); j++) {
        pk.push_back(stablePrimaryKeyColumns[i][j]->element(stableScanIndexTracker[i]).value()->value<int32_t>().value());
      }

      if (compare_primary_key(pk, currPK) < 0) {  // find a smaller PK
        currPK = pk;
        minPKPosition = {i, stableScanIndexTracker[i]};
      }
    }

    for (int i = 0; i < deltaScanIndexTracker.size(); i++) {
      if (deltaScanIndexTracker[i] >= deltaSizes[i])
        continue;
      std::vector<int32_t> pk;
      for (int j = 0; j < primaryKeyColumnNames.size(); j++) {
        pk.push_back(deltaPrimaryKeyColumns[i][j]->element(deltaScanIndexTracker[i]).value()->value<int32_t>().value());
      }

      if (compare_primary_key(pk, currPK) < 0) {  // find a smaller PK
        currPK = pk;
        minPKPosition = {i, deltaScanIndexTracker[i]};
      }
    }
    // now we know the minPK = currPK
    long minTimeStamp = 0;
    std::array<int, 2> position = {0, 0}; // [deltaNum, idx]

    bool deleteFlag = false;  // determine whether we want to delete the record with this PK

    // try to find match key in stable, if we find it, we record its position.
    for (size_t i = 0; i < stableScanIndexTracker.size(); i++) {
      if (stableScanIndexTracker[i] >= stableSizes[i])
        continue;
      std::vector<int32_t> pk;
      for (int j = 0; j < primaryKeyColumnNames.size(); j++) {
        pk.push_back(stablePrimaryKeyColumns[i][j]->element(stableScanIndexTracker[i]).value()->value<int32_t>().value());
      }
      if (compare_primary_key(currPK, pk) == 0) {
        position[0] = i;
        position[1] = stableScanIndexTracker[i];
        stableScanIndexTracker[i] += 1;
      }
    }
    // try to find match key in delta
    for (size_t i = 0; i < deltaScanIndexTracker.size(); i++) {
      if (deltaScanIndexTracker[i] >= deltaSizes[i])
        continue;
      std::vector<int32_t> pk;
      for (int j = 0; j < primaryKeyColumnNames.size(); j++) {
        pk.push_back(deltaPrimaryKeyColumns[i][j]->element(deltaScanIndexTracker[i]).value()->value<int32_t>().value());
      }
      if (compare_primary_key(currPK, pk) == 0) {
        // if we find it
        auto currTS = deltaTimeStampColumns[i];
        if (currTS > minTimeStamp) {  // if the current record is newer
          string type = deltaTypeColumns[i]->element(deltaScanIndexTracker[i]).value()->value<string>().value();
          if (type == "DELETE") {
            deleteFlag = true;
          } else if (type == "INSERT") {
            deleteFlag = false;
            counter++;
          }
          minTimeStamp = currTS;
          position = {static_cast<int>(stableScanIndexTracker.size() + i), deltaScanIndexTracker[i]};
        }
        deltaScanIndexTracker[i] += 1;
      }
    }
    // if the record eventually is not getting deleted
    if (!deleteFlag) {
      if(position[0] == 0){
        stablePositions.push_back(position[1]);
        stableDm.emplace(position[1], position[1]);
      }
      else if(position[0]==deltaScanIndexTracker.size()+1){
        deltasPositions.push_back(position[1]+deltaSizes[0]+deltaSizes[1]);
      }
      else if(position[0]==deltaScanIndexTracker.size()){
        deltasPositions.push_back(position[1]);
      }
      else{
        deltasPositions.push_back(position[1]);
      }
    }
  }

  // delete the keys that the k-way merge algorithm extracted as deleted from the stable keep map
  if(stablePrimaryKeyColumns[0].size()>1) {
    for (int s = 0; s < stables[0]->numRows(); s++) {
      if (stableDm.find(s) == stableDm.end()) {
        stableKm.erase(std::make_pair(stablePrimaryKeyColumns[0][0]->element(s).value()->value<int32_t>().value(),
                                      stablePrimaryKeyColumns[0][1]->element(s).value()->value<int32_t>().value()));
        stableBm[s] = 0;
      }
    }
  }
  else {
    for (int s = 0; s < stables[0]->numRows(); s++) {
      if (stableDm.find(s) == stableDm.end()) {
        stableKm.erase(std::make_pair(stablePrimaryKeyColumns[0][0]->element(s).value()->value<int32_t>().value(),0));
        stableBm[s] = 0;
      }
    }
  }

  return std::tuple(deltasPositions, stableBm);
}