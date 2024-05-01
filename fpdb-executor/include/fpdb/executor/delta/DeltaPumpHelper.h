//
// Created by ZhangOscar on 3/18/22.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_DELTA_DELTAPUMPHELPER_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_DELTA_DELTAPUMPHELPER_H

#include <fpdb/tuple/TupleSet.h>
#include <fpdb/catalogue/Partition.h>
#include <deltapump/makeTuple.h>
#include "../../../../../fpdb-logparser/cpp/include/deltapump/BinlogParser.h"
#include <memory>
#include <unordered_map>

using namespace fpdb::tuple;
using namespace fpdb::catalogue;
using namespace fpdb::logparser;

using Delta = std::unordered_map<int, std::shared_ptr<TupleSet>>;
using SchemaMap = std::unordered_map<std::string, std::shared_ptr<::arrow::Schema>>;

// FIXME: Hard-coded table schema types for initialization
using LineOrderMap = std::unordered_map<int, std::set<struct lineorder_record>>;
using DateMap = std::unordered_map<int, std::set<struct date_record>>;
using PartMap = std::unordered_map<int, std::set<struct part_record>>;
using CustomerMap = std::unordered_map<int, std::set<struct customer_record>>;
using SupplierMap = std::unordered_map<int, std::set<struct supplier_record>>;
using HistoryMap = std::unordered_map<int, std::set<struct history_record>>;

namespace fpdb::executor::delta {

class DeltaPumpHelper {
public:
  /**
   * Pump all current deltas from DeltaPump
   * @param allSchema map of table name to arrow schema of that table
   * @return a map of table name to deltas for all the partitions of that table.
   */
  static std::pair<std::unordered_map<std::string, Delta>, long> pump(SchemaMap allSchema, std::unordered_map<std::string , long> partitionInfo);

private:
  static Delta dispatch(LineOrderMap *record_ptr, std::shared_ptr<::arrow::Schema> schema);
  static Delta dispatch(DateMap *record_ptr, std::shared_ptr<::arrow::Schema> schema);
  static Delta dispatch(PartMap *record_ptr, std::shared_ptr<::arrow::Schema> schema);
  static Delta dispatch(CustomerMap *record_ptr, std::shared_ptr<::arrow::Schema> schema);
  static Delta dispatch(SupplierMap *record_ptr, std::shared_ptr<::arrow::Schema> schema);

  // Below are the helper functions to do row-to-column conversion.
  static std::shared_ptr<TupleSet> rowToColumn(const std::vector<LineorderDelta_t>& deltaTuples, std::shared_ptr<::arrow::Schema> schema);
  static std::shared_ptr<TupleSet> rowToColumn(const std::vector<DateDelta_t>& deltaTuples, std::shared_ptr<::arrow::Schema> schema);
  static std::shared_ptr<TupleSet> rowToColumn(const std::vector<SupplierDelta_t>& deltaTuples, std::shared_ptr<::arrow::Schema> schema);
  static std::shared_ptr<TupleSet> rowToColumn(const std::vector<CustomerDelta_t>& deltaTuples, std::shared_ptr<::arrow::Schema> schema);
  static std::shared_ptr<TupleSet> rowToColumn(const std::vector<PartDelta_t>& deltaTuples, std::shared_ptr<::arrow::Schema> schema);
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_DELTA_DELTAPUMPHELPER_H
