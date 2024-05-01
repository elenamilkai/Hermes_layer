//
// Created by Yifei Yang on 5/26/22.
//

#include <iostream>
#include <arrow/api.h>
#include "../../fpdb-tuple/include/fpdb/tuple/arrow/Arrays.h"
#include "../../fpdb-tuple/include/fpdb/tuple/TupleSet.h"
#include "../../fpdb-expression-gandiva/include/fpdb/expression/gandiva/Filter.h"
using namespace fpdb::tuple;

int main() {

  auto schema = ::arrow::schema({{field("f0", ::arrow::int32())},
                                 {field("f1", ::arrow::int32())},
                                 {field("f2", ::arrow::int32())}});
  auto array_0 = Arrays::make<::arrow::Int32Type>({6, 7, 8, 9, 10}).value();
  auto array_1 = Arrays::make<::arrow::Int32Type>({15, 16, 17, 18, 19}).value();
  auto array_2 = Arrays::make<::arrow::Int32Type>({25, 26, 27, 28, 29}).value();
  arrow::ArrayVector arrayVector{array_0, array_1, array_2};
  auto tupleSet1 = TupleSet::make(schema, arrayVector);
  std::cout << "Original tupleSet1:" << std::endl;
  std::cout << tupleSet1->showString(TupleSetShowOptions(TupleSetShowOrientation::RowOriented)) << std::endl;


  auto array_3 = Arrays::make<::arrow::Int32Type>({1, 2, 3, 4, 5}).value();
  auto array_4 = Arrays::make<::arrow::Int32Type>({10, 11, 12, 13, 14}).value();
  auto array_5 = Arrays::make<::arrow::Int32Type>({20, 21, 22, 23, 24}).value();
  arrow::ArrayVector arrayVector2{array_3, array_4, array_5};
  auto tupleSet2 = TupleSet::make(schema, arrayVector2);
  std::cout << "Original tupleSet2:" << std::endl;
  std::cout << tupleSet2->showString(TupleSetShowOptions(TupleSetShowOrientation::RowOriented)) << std::endl;

  auto combinedMemoryDelta = TupleSet::concatenate({tupleSet1});
  combinedMemoryDelta = TupleSet::concatenate({combinedMemoryDelta.value(), tupleSet2});
  combinedMemoryDelta.value()->combine();

  std::cout << "Combined tupleSet:" << std::endl;
  std::cout << combinedMemoryDelta.value()->showString() <<  std::endl;

  arrow::Status arrowStatus;
  std::shared_ptr<arrow::RecordBatch> batch;
  auto arrowTable = combinedMemoryDelta.value()->table();
  arrow::TableBatchReader reader(*arrowTable);
  reader.set_chunksize((int64_t) combinedMemoryDelta.value()->numRows()); // we read the entire table as one chunk, might be problematic later
  arrowStatus = reader.ReadNext(&batch);
  std::cout << "Batch:" << std::endl;
  std::cout << batch->ToString() << std::endl;

  if (!arrowStatus.ok()) {
    throw std::runtime_error(arrowStatus.message());
  }

  std::shared_ptr<::gandiva::SelectionVector> selectionVector;
  auto status = ::gandiva::SelectionVector::MakeInt64(combinedMemoryDelta.value()->numRows(), ::arrow::default_memory_pool(), &selectionVector);
  if (!status.ok()) {
    throw std::runtime_error(status.message());
  }
  selectionVector->SetNumSlots(10);
  selectionVector->SetIndex(0, 9);
  selectionVector->SetIndex(1, 7);
  selectionVector->SetIndex(2, 5);
  selectionVector->SetIndex(3, 3);
  selectionVector->SetIndex(4, 1);
  selectionVector->SetIndex(5, 2);
  selectionVector->SetIndex(6, 8);
  selectionVector->SetIndex(7, 4);
  selectionVector->SetIndex(8, 6);
  selectionVector->SetIndex(9, 0);
  auto expOutputArrays = fpdb::expression::gandiva::Filter::evaluateBySelectionVectorStatic(*batch, selectionVector);
  if (!expOutputArrays.has_value()) {
    throw std::runtime_error(expOutputArrays.error());
  }
  auto projectTupleSet = TupleSet::make(schema, *expOutputArrays);
  std::cout << "Project tupleSet:" << std::endl;
  std::cout << projectTupleSet->showString(TupleSetShowOptions(TupleSetShowOrientation::RowOriented)) << std::endl;

      return 0;
}
