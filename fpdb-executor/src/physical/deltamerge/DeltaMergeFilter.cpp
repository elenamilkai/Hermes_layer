//
// Created by Han Cao on 3/20/22.
//

#include "fpdb/executor/physical/deltamerge/DeltaMergeFilter.h"
#include "fpdb/tuple/Util.h"
#include <gandiva/tree_expr_builder.h>
#include <fpdb/delta/DeltaCacheMetadata.h>

using namespace fpdb::executor::physical::deltamerge;

void DeltaMergeFilter::SetBit(uint8_t* bits, int64_t i) {
  uint8_t kBitmask[] = {1, 2, 4, 8, 16, 32, 64, 128};
  bits[i / 8] |= kBitmask[i % 8];
}

int DeltaMergeFilter::roundUp(int numToRound, int multiple){
  if (multiple == 0)
    return numToRound;

  int remainder = numToRound % multiple;
  if (remainder == 0)
    return numToRound;

  return numToRound + multiple - remainder;
}
/**
 * Constructor for selection vector
 * @param keepMap a set that contains all the indexes of rows
 * @param batch_size size of the selection vector
 * @return the selection vector for the arrow table
 */
tl::expected<std::shared_ptr<gandiva::SelectionVector>, std::string>
DeltaMergeFilter::generateSelectionVector(const std::vector<int> keepPosDelta,
                                          std::shared_ptr<std::vector<uint8_t>> stableBm) {
  std::shared_ptr<gandiva::SelectionVector> selectionVectorD; // selection vector of the deltas
  std::shared_ptr<gandiva::SelectionVector> selectionVectorS;  // selection vector generated from the stables' bitMap
  // if the stable bitMap is not null then generate the selection vector of the stables from the bitMap
  if(stableBm!= nullptr) {
    int max_slots = stableBm->size();
    int bitmap_size = roundUp(max_slots, 64) * 8;
    std::vector<uint8_t> bitmap(bitmap_size);
    for (int j = 0; j < stableBm->size(); j++) {
      if ((*stableBm)[j] == 1) {
        SetBit(&bitmap[0], j);
      }
    }
    auto statusS = ::gandiva::SelectionVector::MakeInt64(max_slots, ::arrow::default_memory_pool(), &selectionVectorS);
    if (!statusS.ok()) {
      return tl::make_unexpected(statusS.message());
    }
    statusS = selectionVectorS->PopulateFromBitMap(&bitmap[0], bitmap_size, max_slots - 1);
    if (!statusS.ok()) {
      return tl::make_unexpected(statusS.message());
    }
    return selectionVectorS;
  }
  // if the keepPositions of the delta is not empty then generate selection vector from the keepMap
  else if(!keepPosDelta.empty()) {
    auto status = ::gandiva::SelectionVector::MakeInt64(keepPosDelta.size(), ::arrow::default_memory_pool(), &selectionVectorD);
    if (!status.ok()) {
      return tl::make_unexpected(status.message());
    }
    selectionVectorD->SetNumSlots(keepPosDelta.size());
    int count = 0;
    for (auto idx_value : keepPosDelta) {
      selectionVectorD->SetIndex(count, idx_value);
      count++;
    }
    return selectionVectorD;
  }
}

/**
 * Using the selection vector to evaluate the tuple set
 * @param keepMap a set that contains all the indexes of rows
 * @param tupleSet input table
 * @return filtered tuple set
 */
tl::expected<std::shared_ptr<fpdb::tuple::TupleSet>, std::string>
DeltaMergeFilter::evaluate(const std::vector<int> keepPosDelta,
                           std::shared_ptr<std::vector<uint8_t>> stableBm,
                           std::shared_ptr<fpdb::tuple::TupleSet> tupleSet,
                           std::vector<std::string> projectColumnNames,
                           bool predicatesExist,
                           std::string tableName) {

  if(tupleSet->numRows()==0){
    return tupleSet;
  }
  std::vector<int> batchStartPos;
  auto arrowTable = tupleSet->table();
  auto filteredTupleSet = fpdb::tuple::TupleSet::make(tupleSet->schema());
  auto schema = filteredTupleSet->schema();

  std::shared_ptr<::gandiva::Projector> gandivaProjector;
  std::vector<std::shared_ptr<::gandiva::Expression>> fieldExpressions;
  std::shared_ptr<gandiva::SelectionVector> selectionVector;
  for (const auto &field: schema->fields()) {
    auto gandivaField = ::gandiva::TreeExprBuilder::MakeField(field);
    auto fieldExpression = ::gandiva::TreeExprBuilder::MakeExpression(gandivaField, field);
    fieldExpressions.push_back(fieldExpression);
  }
  arrow::Status arrowStatus;
  std::shared_ptr<arrow::RecordBatch> batch;
  arrow::TableBatchReader reader(*arrowTable);
  reader.set_chunksize((int64_t)fpdb::tuple::DefaultChunkSize);
  arrowStatus = reader.ReadNext(&batch);
  if (!arrowStatus.ok()) {
    return tl::make_unexpected(arrowStatus.message());
  }
  int64_t startPos = 0;
  int64_t endPos = batch->num_rows() - 1;
  int64_t seenRows = 0;
  int size_bm  = 0;
  int size_kp = 0;
  while (batch != nullptr) {
    assert(batch->ValidateFull().ok());
    auto batchRows = batch->num_rows();
    if(stableBm!= nullptr) {
      std::vector<uint8_t> batchStableBm(stableBm->begin()+startPos, stableBm->begin()+startPos+batchRows);
      selectionVector = generateSelectionVector(keepPosDelta, std::make_shared<std::vector<uint8_t>>(batchStableBm)).value();
      size_bm = batchStableBm.size();
    }
    else if(!keepPosDelta.empty()) {
      auto start = std::lower_bound(keepPosDelta.begin(), keepPosDelta.end(), startPos);
      auto end = std::upper_bound(keepPosDelta.begin(), keepPosDelta.end(), endPos);
      std::vector<int> batchKeepPosDelta(std::distance(keepPosDelta.begin(), end - 1) - std::distance(keepPosDelta.begin(), start) + 1);
      std::copy(start, end, batchKeepPosDelta.begin());
      std::transform(batchKeepPosDelta.begin(), batchKeepPosDelta.end(), batchKeepPosDelta.begin(),
                     bind2nd(std::minus<int>(), seenRows));
      seenRows += batchRows;
      selectionVector = generateSelectionVector(batchKeepPosDelta, stableBm).value();
      size_kp = batchKeepPosDelta.size();
    }
    startPos += batchRows;
    auto status = ::gandiva::Projector::Make(schema,
                                             fieldExpressions,
                                             selectionVector->GetMode(),
                                             ::gandiva::ConfigurationBuilder::DefaultConfiguration(),
                                             &gandivaProjector);

    if (!status.ok()) {
      return tl::make_unexpected(status.message());
    }
    std::shared_ptr<::arrow::Table> batchArrowTable;
    arrow::ArrayVector outputs;
    if (selectionVector->GetNumSlots() > 0) {
      auto status = gandivaProjector->Evaluate(*batch, selectionVector.get(), arrow::default_memory_pool(), &outputs);
      if (!status.ok()) {
        return tl::make_unexpected(status.message());
      }
      batchArrowTable = ::arrow::Table::Make(schema, outputs);
    } else {
      auto columns = fpdb::tuple::Schema::make(schema)->makeColumns();
      auto arrowArrays = fpdb::tuple::Column::columnVectorToArrowChunkedArrayVector(columns);
      batchArrowTable = ::arrow::Table::Make(schema, arrowArrays);
    }
    // construct the result
    auto batchTupleSet = std::make_shared<fpdb::tuple::TupleSet>(batchArrowTable);
    auto result = filteredTupleSet->append(batchTupleSet);
    arrowStatus = reader.ReadNext(&batch);
    if (!arrowStatus.ok()) {
      return tl::make_unexpected(arrowStatus.message());
    }
    if(batch != nullptr) {
      endPos += batch->num_rows();
    }

  }
  return filteredTupleSet;
}
