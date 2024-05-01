//
// Created by Han on 18/02/22.
//

#include <fpdb/tuple/ColumnBuilder.h>
#include <arrow/type.h>
#include <fpdb/executor/physical/deltamerge/ResultGeneratorKernel.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace fpdb::executor::physical::deltamerge;
using namespace fpdb::tuple;

std::shared_ptr<TupleSet> ResultGeneratorKernel::getFinalResult(
    std::vector<std::shared_ptr<DeltaCacheData>> deltas,
    std::vector<std::shared_ptr<TupleSet>> stables,
    std::vector<std::shared_ptr<TupleSet>> diskDeltas,
    std::vector<int> diskDeltasTimestamps,
    std::unordered_map<int, std::set<int>> deleteMap,
    std::vector<std::string> &projectColumnNames,
    std::shared_ptr<fpdb::catalogue::Table> table) {

  size_t numOfOutputColumns = projectColumnNames.size();
  std::vector<std::shared_ptr<ColumnBuilder>> columnBuilderArray(numOfOutputColumns);
  std::shared_ptr<arrow::Schema> schema = table->getSchema();

  // Populate column names with the corresponding type
  std::unordered_map<std::string, std::shared_ptr<arrow::DataType>> nameToTypeMap;
  for (const std::string &colName : projectColumnNames) {
    nameToTypeMap[colName] = schema->GetFieldByName(colName)->type();
  }

  // Initialize the column builder
  for (size_t i = 0; i < numOfOutputColumns; i++) {
    std::string newColumnBuilderName = projectColumnNames[i];
    auto newColumnBuilder = ColumnBuilder::make(newColumnBuilderName, nameToTypeMap[projectColumnNames[i]]);
    columnBuilderArray[i] = newColumnBuilder;
  }

  // append the stable data to the final output first
  for (size_t i = 0; i < stables.size(); i++) {
    std::set<int> deleteSetStable = {};
    //TODO:: bring this back with composite primary keys
    if (deleteMap.find(i) != deleteMap.end()) {  // find the deleteSet from the deleteMap.
      deleteSetStable = deleteMap[i];
    }
    auto originalStableTable = stables[i];
    for (size_t c = 0; c < numOfOutputColumns; c++) {
      if (c == 0)
        SPDLOG_CRITICAL("Stable data number of rows: {}", originalStableTable->numRows());
      auto curColumn = originalStableTable->getColumnByName(projectColumnNames[c]).value();
      for (int64_t r = 0; r < originalStableTable->numRows(); r++) {
        // ignore the rows in delete set
        //TODO:: bring this back with composite primary keys
        if (!deleteSetStable.count(r))
          continue;
        columnBuilderArray[c]->append(curColumn->element(r).value());
      }

    }
  }

  // append all the delta data to the final output
  for (size_t j = 0; j < deltas.size(); j++) {
    int offset_i = j + stables.size();
    std::set<int> deleteSetDelta = {};
    //TODO:: bring this back with composite primary keys
    if (deleteMap.find(offset_i) != deleteMap.end()) {  // find the deleteSet from the deleteMap
      deleteSetDelta = deleteMap[offset_i];
    }
    auto originalDeltaTable = deltas[j]->getDelta();
    for (size_t c = 0; c < numOfOutputColumns; c++) {
      auto curColumn = originalDeltaTable->getColumnByName(projectColumnNames[c]).value();
      if (c == 0)
        SPDLOG_CRITICAL("Memory Deltas number of rows: {}", originalDeltaTable->numRows());
      for (int64_t r = 0; r < originalDeltaTable->numRows(); r++) {
        //TODO:: bring this back with composite primary keys
        if (!deleteSetDelta.count(r))
          continue;
//            auto columnType = nameToTypeMap[projectColumnNames[c]];
        columnBuilderArray[c]->append(curColumn->element(r).value());
//            if (curColumn->type()->Equals(::arrow::int32()) && columnType->Equals(::arrow::int64()) ) {
//              // unmatched type in schema, we just hardcoded to transform the int32 into int64
//              int32_t originalVal = curColumn->element(r).value()->value<int32_t>().value();
//              auto convertedVal = (int64_t) originalVal;
//              auto appendingScalar = ::arrow::MakeScalar(arrow::int64(), convertedVal).ValueOrDie();
//              columnBuilderArray[c]->append(Scalar::make(appendingScalar));
//            } else {
//              columnBuilderArray[c]->append(curColumn->element(r).value());
//            }
      }
    }
  }

  // append all the disk deltas to the final output
  if (diskDeltas.size() != 0) {
    for (size_t j = 0; j < diskDeltas.size(); j++) {
      int offset_i = j + stables.size() + deltas.size();
      std::set<int> deleteSetDiskDeltas = {};
      //TODO:: bring this back after debbuging deleteMap
      if (deleteMap.find(offset_i) != deleteMap.end())
        deleteSetDiskDeltas = deleteMap[offset_i];
      auto originalDiskDeltaTable = diskDeltas[j];
      for (size_t c = 0; c < numOfOutputColumns; c++) {
        auto curColumn = originalDiskDeltaTable->getColumnByName(projectColumnNames[c]).value();
        for (int64_t r = 0; r < originalDiskDeltaTable->numRows(); r++) {
          //TODO:: bring this back after debbuging deleteMap
          if (deleteSetDiskDeltas.count(r))
            continue;
          columnBuilderArray[c]->append(curColumn->element(r).value());
        }
      }
    }
  }

  // Make final results
  std::vector<std::shared_ptr<Column>> builtColumns;
  builtColumns.reserve(columnBuilderArray.size());
  for (auto &colBuilder : columnBuilderArray) {
    const auto expColumn = colBuilder->finalize();
    if (!expColumn.has_value()) {
      throw runtime_error(fmt::format("DeltaMerge: failed to build column!"));
    }
    builtColumns.emplace_back(expColumn.value()); // TODO: Proper error handling with tl::expected
  }
  auto finalOutput = TupleSet::make(builtColumns);
  SPDLOG_CRITICAL("Final output result: {}", finalOutput->numRows());
  return finalOutput;
}