//
// Created by ZhangOscar on 3/18/22.
//

#include <fpdb/executor/delta/DeltaPumpHelper.h>
#include "../../../../../fpdb-logparser/cpp/include/deltapump/BinlogParser.h"
#include <deltapump/makeTuple.h>
#include <arrow/stl.h>
#include <set>

using namespace fpdb::executor::delta;
using namespace fpdb::logparser;

thread_local std::unique_ptr<BinlogParser> binlogParser = std::make_unique<BinlogParser>();

std::pair<std::unordered_map<std::string, Delta>, long>
    DeltaPumpHelper::pump(SchemaMap allSchema, std::unordered_map<std::string , long> partitionInfo) {
  LineOrderMap *lineorder_record_ptr = nullptr;
  DateMap *date_record_ptr = nullptr;
  PartMap *part_record_ptr = nullptr;
  CustomerMap *customer_record_ptr = nullptr;
  SupplierMap *supplier_record_ptr = nullptr;
  HistoryMap *history_record_ptr = nullptr;

  auto timestamp = binlogParser->parse(&lineorder_record_ptr,
                     &customer_record_ptr,
                     &supplier_record_ptr,
                     &part_record_ptr,
                     &date_record_ptr,
                     &history_record_ptr,
                     partitionInfo);



  if (lineorder_record_ptr == nullptr ||
      customer_record_ptr == nullptr ||
      supplier_record_ptr == nullptr ||
      part_record_ptr == nullptr ||
      date_record_ptr == nullptr) {
    throw std::runtime_error(fmt::format("Error parsing binlog resulting some delta result to be null!"));
  }

  const int numTable = 5;
  std::unordered_map<std::string, Delta> allDeltas(numTable);
  allDeltas.insert({
    { "lineorder", DeltaPumpHelper::dispatch(lineorder_record_ptr, allSchema["lineorder"]) },
    { "date", DeltaPumpHelper::dispatch(date_record_ptr, allSchema["date"]) },
    { "part", DeltaPumpHelper::dispatch(part_record_ptr, allSchema["part"]) },
    { "customer", DeltaPumpHelper::dispatch(customer_record_ptr, allSchema["customer"]) },
    { "supplier", DeltaPumpHelper::dispatch(supplier_record_ptr, allSchema["supplier"]) }
  }); // FIXME: remove hardcoded table names

  return std::pair(allDeltas, timestamp);
}

Delta DeltaPumpHelper::dispatch(LineOrderMap *record_ptr, std::shared_ptr<::arrow::Schema> schema) {
  // Fetch and convert all lineorder deltas
  std::unordered_map<int, std::shared_ptr<TupleSet>> lineOrderDeltaRecords;
  for (const auto& partition_pair : *record_ptr) {
    int partition = partition_pair.first;
    std::set<struct lineorder_record> record_set = partition_pair.second;
    std::vector<LineorderDelta_t> record_table(0);
    for (const auto &record: record_set) {
      record_table.emplace_back(record.lineorder_delta);
    }
    std::shared_ptr<TupleSet> column_table = DeltaPumpHelper::rowToColumn(record_table, schema);
    lineOrderDeltaRecords[partition] = column_table;
  }
  delete record_ptr;
  return lineOrderDeltaRecords;
}

Delta DeltaPumpHelper::dispatch(DateMap *record_ptr, std::shared_ptr<::arrow::Schema> schema) {
  // Fetch and convert all date deltas
  std::unordered_map<int, std::shared_ptr<TupleSet>> dateDeltaRecords;
  for (const auto& partition_pair : *record_ptr) {
    int partition = partition_pair.first;
    std::set<struct date_record> record_set = partition_pair.second;
    std::vector<DateDelta_t> record_table(0);
    for (const auto &record: record_set) {
      record_table.emplace_back(record.date_delta);
    }
    std::shared_ptr<TupleSet> column_table = DeltaPumpHelper::rowToColumn(record_table, schema);
    dateDeltaRecords[partition] = column_table;
  }
  delete record_ptr;
  return dateDeltaRecords;
}

Delta DeltaPumpHelper::dispatch(PartMap *record_ptr, std::shared_ptr<::arrow::Schema> schema) {
  // Fetch and convert all part deltas
  std::unordered_map<int, std::shared_ptr<TupleSet>> partDeltaRecords;
  for (const auto& partition_pair : *record_ptr) {
    int partition = partition_pair.first;
    std::set<struct part_record> record_set = partition_pair.second;
    std::vector<PartDelta_t> record_table(0);
    for (const auto &record: record_set) {
      record_table.emplace_back(record.part_delta);
    }
    std::shared_ptr<TupleSet> column_table = DeltaPumpHelper::rowToColumn(record_table, schema);
    partDeltaRecords[partition] = column_table;
  }
  delete record_ptr;
  return partDeltaRecords;
}

Delta DeltaPumpHelper::dispatch(CustomerMap *record_ptr, std::shared_ptr<::arrow::Schema> schema) {
  // Fetch and convert all customer deltas
  std::unordered_map<int, std::shared_ptr<TupleSet>> customerDeltaRecords;
  for (const auto& partition_pair : *record_ptr) {
    int partition = partition_pair.first;
    std::set<struct customer_record> record_set = partition_pair.second;
    std::vector<CustomerDelta_t> record_table(0);
    for (const auto &record: record_set) {
      record_table.emplace_back(record.customer_delta);
    }
    std::shared_ptr<TupleSet> column_table = DeltaPumpHelper::rowToColumn(record_table, schema);
    customerDeltaRecords[partition] = column_table;
  }
  delete record_ptr;
  return customerDeltaRecords;
}

Delta DeltaPumpHelper::dispatch(SupplierMap *record_ptr, std::shared_ptr<::arrow::Schema> schema) {
  // Fetch and convert all supplier deltas
  std::unordered_map<int, std::shared_ptr<TupleSet>> supplierDeltaRecords;
  for (const auto& partition_pair : *record_ptr) {
    int partition = partition_pair.first;
    std::set<struct supplier_record> record_set = partition_pair.second;
    std::vector<SupplierDelta_t> record_table(0);
    for (const auto &record: record_set) {
      record_table.emplace_back(record.supplier_delta);
    }
    std::shared_ptr<TupleSet> column_table = DeltaPumpHelper::rowToColumn(record_table, schema);
    supplierDeltaRecords[partition] = column_table;
  }
  delete record_ptr;
  return supplierDeltaRecords;
}

std::shared_ptr<TupleSet> DeltaPumpHelper::rowToColumn(const std::vector<LineorderDelta_t> &deltaTuples,
                                                       std::shared_ptr<::arrow::Schema> schema) {

  auto pool = ::arrow::default_memory_pool();
  std::shared_ptr<::arrow::Table> table;
  auto valid = arrow::stl::TableFromTupleRange(pool, deltaTuples, schema->field_names(), &table).ok();
  if (!valid) {
    throw runtime_error(fmt::format("Error in row-to-col conversion for table `LineOrder`."));
  }
  auto res = fpdb::tuple::TupleSet::make(schema, table->columns());
  return res;
}

std::shared_ptr<TupleSet> DeltaPumpHelper::rowToColumn(const std::vector<DateDelta_t>& deltaTuples,
                                                       std::shared_ptr<::arrow::Schema> schema) {
  auto pool = ::arrow::default_memory_pool();
  std::shared_ptr<::arrow::Table> table;
  auto valid = arrow::stl::TableFromTupleRange(pool, deltaTuples, schema->field_names(), &table).ok();
  if (!valid) {
    throw runtime_error(fmt::format("Error in row-to-col conversion for table `Date`."));
  }
  return fpdb::tuple::TupleSet::make(schema, table->columns());
}

std::shared_ptr<TupleSet> DeltaPumpHelper::rowToColumn(const std::vector<PartDelta_t> &deltaTuples,
                                                       std::shared_ptr<::arrow::Schema> schema) {
  auto pool = ::arrow::default_memory_pool();
  std::shared_ptr<::arrow::Table> table;
  auto valid = arrow::stl::TableFromTupleRange(pool, deltaTuples, schema->field_names(), &table).ok();
  if (!valid) {
    throw runtime_error(fmt::format("Error in row-to-col conversion for table `Part`."));
  }
  return fpdb::tuple::TupleSet::make(schema, table->columns());
}

std::shared_ptr<TupleSet> DeltaPumpHelper::rowToColumn(const std::vector<CustomerDelta_t> &deltaTuples,
                                                       std::shared_ptr<::arrow::Schema> schema) {
  auto pool = ::arrow::default_memory_pool();
  std::shared_ptr<::arrow::Table> table;

  auto valid = arrow::stl::TableFromTupleRange(pool, deltaTuples, schema->field_names(), &table).ok();
  if (!valid) {
    throw runtime_error(fmt::format("Error in row-to-col conversion for table `Customer`."));
  }
  //auto f = TupleSet::make(table);
  return fpdb::tuple::TupleSet::make(schema, table->columns());
}

std::shared_ptr<TupleSet> DeltaPumpHelper::rowToColumn(const std::vector<SupplierDelta_t> &deltaTuples,
                                                       std::shared_ptr<::arrow::Schema> schema) {
  auto pool = ::arrow::default_memory_pool();
  std::shared_ptr<::arrow::Table> table;
  auto valid = arrow::stl::TableFromTupleRange(pool, deltaTuples, schema->field_names(), &table).ok();
  if (!valid) {
    throw runtime_error(fmt::format("Error in row-to-col conversion for table `Supplier`."));
  }
  return fpdb::tuple::TupleSet::make(schema, table->columns());
}
