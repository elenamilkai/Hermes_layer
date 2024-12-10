//
// Created by matt on 12/12/19.
//

#include <fpdb/tuple/TupleSet.h>

#include <utility>
#include <sstream>
#include <cassert>
#include <cstdlib>                      // for abort
#include <arrow/scalar.h>
#include <iomanip>

namespace arrow { class MemoryPool; }

using namespace fpdb::tuple;

TupleSet::TupleSet(std::shared_ptr<arrow::Table> table) :
  table_(std::move(table)) {}

std::shared_ptr<TupleSet> TupleSet::make(const std::shared_ptr<arrow::Table> &table) {
  return std::make_shared<TupleSet>(table);
}

std::shared_ptr<TupleSet> TupleSet::make(const std::vector<std::shared_ptr<Column>>& columns) {
  std::vector<std::shared_ptr<::arrow::ChunkedArray>> arrays;
  std::vector<std::shared_ptr<::arrow::Field>> fields;
  for(const auto &column: columns){
    arrays.push_back(column->getArrowArray());
    fields.push_back(std::make_shared<::arrow::Field>(column->getName(), column->type()));
  }
  auto schema = std::make_shared<::arrow::Schema>(fields);
  return make(schema, arrays);
}

std::shared_ptr<TupleSet> TupleSet::make(const std::shared_ptr<arrow::Schema>& schema) {
  auto columns = Schema::make(schema)->makeColumns();
  return make(columns);
}

std::shared_ptr<TupleSet> TupleSet::make(const std::shared_ptr<arrow::Schema>& schema,
                                         const std::vector<std::shared_ptr<arrow::Array>>& values){
  auto table = arrow::Table::Make(schema, values);
  return std::make_shared<TupleSet>(table);
}

std::shared_ptr<TupleSet> TupleSet::make(const std::shared_ptr<arrow::Schema>& schema,
                                         const std::vector<std::shared_ptr<arrow::ChunkedArray>>& values){
  auto table = arrow::Table::Make(schema, values);
  return std::make_shared<TupleSet>(table);
}

std::shared_ptr<TupleSet> TupleSet::make(const std::shared_ptr<arrow::Schema>& schema,
                                         const std::vector<std::shared_ptr<Column>>& columns) {
  auto chunkedArrays = Column::columnVectorToArrowChunkedArrayVector(columns);
  auto arrowTable = ::arrow::Table::Make(schema, chunkedArrays);
  return std::make_shared<TupleSet>(arrowTable);
}

tl::expected<std::shared_ptr<TupleSet>, std::string>
TupleSet::make(const std::shared_ptr<arrow::csv::TableReader> &tableReader) {
  auto result = tableReader->Read();
  if (!result.ok()) {
    return tl::make_unexpected(result.status().message());
  }

  auto tupleSet = std::make_shared<TupleSet>();
  auto table = result.ValueOrDie();
  tupleSet->table_ = table;

  assert(tupleSet);
  assert(tupleSet->table_);
  assert(tupleSet->table_->ValidateFull().ok());

  return tupleSet;
}

std::shared_ptr<TupleSet> TupleSet::makeWithNullTable() {
  return std::make_shared<TupleSet>();
}

std::shared_ptr<TupleSet> TupleSet::makeWithEmptyTable() {
  std::vector<std::shared_ptr<arrow::Field>> fields;
  auto schema = arrow::schema(fields);
  return make(schema);
}

bool TupleSet::valid() const {
  return table_ != nullptr;
}

bool TupleSet::validate() const {
  if(valid())
    return table_->ValidateFull().ok();
  else
    return true;
}

void TupleSet::clear() {
  table_ = nullptr;
}

int64_t TupleSet::numRows() const {
  if (table_) {
    return table_->num_rows();
  } else {
    return 0;
  }
}

int TupleSet::numColumns() const {
  if (table_) {
    return table_->num_columns();
  } else {
    return 0;
  }
}

size_t TupleSet::size() const {
  size_t size = 0;
  for (int i = 0; i < numColumns()-1; i++) {
    size += getColumnByIndex(i).value()->size();
  }
  return size;
}

std::shared_ptr<arrow::Schema> TupleSet::schema() const {
  return table_->schema();
}

std::shared_ptr<arrow::Table> TupleSet::table() const {
  return table_;
}

void TupleSet::table(const std::shared_ptr<arrow::Table> &table) {
  table_ = table;
}

tl::expected<std::shared_ptr<TupleSet>,
        std::string> TupleSet::concatenate(const std::vector<std::shared_ptr<TupleSet>> &tupleSets) {

  // Make sure the tuple sets are valid and have the same schema
  std::shared_ptr<TupleSet> tupleSet1;
  std::vector<std::shared_ptr<TupleSet>> tupleSets1;
  for(const auto & tupleSet : tupleSets){
    if(!tupleSet->valid()){
      return tl::make_unexpected("Cannot concatenate empty tuple sets");
    }
    if(tupleSet1 == nullptr){
      tupleSet1 = tupleSet;
      tupleSets1.emplace_back(tupleSet);
    }
    else{
      /**
       * Need to make two schemas in the same order first, then compare
       */
      if(!tupleSet->table()->schema()->Equals(tupleSet1->table()->schema())){
        // try to reorder the second table according the schema of the first table
        auto reorderedColumns = std::make_shared<std::vector<std::shared_ptr<Column>>>();
        auto schemaMap = std::make_shared<std::unordered_map<std::string, std::shared_ptr<::arrow::Field>>>();
        for (auto const &field: tupleSet->schema()->fields()) {
          schemaMap->insert({field->name(), field});
        }

        for (auto const &field1: tupleSet1->schema()->fields()) {
          if (schemaMap->find(field1->name()) == schemaMap->end()) {
            return tl::make_unexpected("Cannot concatenate tuple sets with different schemas");
          } else {
            reorderedColumns->emplace_back(tupleSet->getColumnByName(field1->name()).value());
          }
        }
        tupleSets1.emplace_back(make(*reorderedColumns));
      } else {
        tupleSets1.emplace_back(tupleSet);
      }
    }
  }

  // Create a vector of arrow tables to concatenate
  std::vector<std::shared_ptr<::arrow::Table>> tableVector = tupleSetVectorToArrowTableVector(tupleSets1);

  // Concatenate
  auto tableResult = arrow::ConcatenateTables(tableVector);
  if (tableResult.ok()) {
    return std::make_shared<TupleSet>(*tableResult);
  } else {
    return tl::make_unexpected(tableResult.status().ToString());
  }
}

tl::expected<void, std::string> TupleSet::append(const std::vector<std::shared_ptr<TupleSet>> &tupleSets) {
  auto tupleSetVector = std::vector{shared_from_this()};
  tupleSetVector.insert(tupleSetVector.end(), tupleSets.begin(), tupleSets.end());
  auto expected = concatenate(tupleSetVector);
  if (!expected.has_value())
    return tl::make_unexpected(expected.error());
  else {
    this->table_ = expected.value()->table_;
    return {}; // TODO: This seems to be how to return a void expected AFAICT, verify?
  }
}

tl::expected<void, std::string> TupleSet::append(const std::shared_ptr<TupleSet> &tupleSet) {
  auto tupleSetVector = std::vector{tupleSet};
  return append(tupleSetVector);
}

tl::expected<std::shared_ptr<Column>, std::string> TupleSet::getColumnByName(const std::string &name) const {
  auto canonicalColumnName = ColumnName::canonicalize(name);
  auto columnArray = table_->GetColumnByName(canonicalColumnName);
  if (columnArray == nullptr) {
    return tl::make_unexpected("Column '" + canonicalColumnName + "' does not exist");
  } else {
    auto column = Column::make(canonicalColumnName, columnArray);
    return column;
  }
}

tl::expected<std::shared_ptr<Column>, std::string> TupleSet::getColumnByIndex(const int &columnIndex) const {
  auto columnName = table_->field(columnIndex)->name();
  auto columnArray = table_->column(columnIndex);
  if (columnArray == nullptr) {
    return tl::make_unexpected("Column '" + std::to_string(columnIndex) + "' does not exist");
  } else {
    auto column = Column::make(columnName, columnArray);
    return column;
  }
}

tl::expected<std::shared_ptr<TupleSet>, std::string>
TupleSet::projectExist(const std::vector<std::string> &columnNames) const {
  std::vector<std::shared_ptr<Column>> columns;
  for (const auto &columnName: columnNames) {
    const auto &expColumn = getColumnByName(columnName);
    if (expColumn.has_value()) {
      columns.emplace_back(expColumn.value());
    }
  }
  if (columns.empty()) {
    return tl::make_unexpected("No column exists.");
  }
  return make(columns);
}

tl::expected<std::shared_ptr<TupleSet>, std::string> TupleSet::project(const std::vector<int> &columnIds) const {
  const auto &expTable = table_->SelectColumns(columnIds);
  if (!expTable.ok()) {
    return tl::make_unexpected(expTable.status().message());
  }
  return make(expTable.ValueOrDie());
}

tl::expected<void, std::string> TupleSet::renameColumns(const std::vector<std::string>& columnNames){
  if(valid()){
    auto expectedTable = table_->RenameColumns(columnNames);
    if(expectedTable.ok())
      table_ = *expectedTable;
    else
      return tl::make_unexpected(expectedTable.status().message());
  }
  return {};
}

tl::expected<void, std::string>
TupleSet::renameColumns(const std::unordered_map<std::string, std::string> &columnRenames) {
  if (valid() && !columnRenames.empty()) {
    auto columnNames = table_->ColumnNames();

    // collect ids
    std::unordered_map<std::string, uint> columnNameIds;
    for (uint i = 0; i < columnNames.size(); ++i) {
      columnNameIds.emplace(columnNames[i], i);
    }

    // rename
    for (const auto &renameIt: columnRenames) {
      const auto &oldName = renameIt.first;
      const auto &newName = renameIt.second;
      const auto &columnIdIt = columnNameIds.find(oldName);
      if (columnIdIt == columnNameIds.end()) {
        return tl::make_unexpected("Column '" + oldName + "' does not exist");
      }
      columnNames[columnIdIt->second] = newName;
    }
    auto expectedTable = table_->RenameColumns(columnNames);
    if(expectedTable.ok())
      table_ = *expectedTable;
    else
      return tl::make_unexpected(expectedTable.status().message());
  }
  return {};
}

tl::expected<void, std::string> TupleSet::combine() {
  auto expectedTable = table_->CombineChunks();
  if(expectedTable.ok())
    table_ = *expectedTable;
  else
    return tl::make_unexpected(expectedTable.status().message());
  return {};
}

std::string TupleSet::showString() {
  return showString(TupleSetShowOptions(TupleSetShowOrientation::ColumnOriented));
}

/**
 * FIXME: This is really slow
 * @param options
 * @return
 */
std::string TupleSet::showString(TupleSetShowOptions options) {

  if (!valid()) {
    return "<empty>";
  } else {

    if (options.getOrientation() == TupleSetShowOrientation::ColumnOriented) {
      auto ss = std::stringstream();
      arrow::Status arrowStatus = ::arrow::PrettyPrint(*table_, 0, &ss);

      if (!arrowStatus.ok()) {
        // FIXME: How to handle this? How can pretty print fail? RAM? Maybe just abort?
        throw std::runtime_error(arrowStatus.detail()->ToString());
      }

      return ss.str();
    } else {

      int width = 120;

      std::stringstream ss;

      ss << std::endl;

      ss << std::left << std::setw(width) << std::setfill('-') << "" << std::endl;

      int columnWidth = table_->num_columns() == 0 ? width : width / table_->num_columns();

      // Column names
      for (const auto &field: table_->schema()->fields()) {
        ss << std::left << std::setw(columnWidth) << std::setfill(' ');
        ss << "| " + field->name();
      }
      ss << std::endl;

      // Column types
      for (const auto &field: table_->schema()->fields()) {
        ss << std::left << std::setw(columnWidth) << std::setfill(' ');
        auto fieldType = field->type();
        if (fieldType) {
          ss << "| " + field->type()->ToString();
        } else {
          ss << "| unknown";
        }
      }
      ss << std::endl;

      ss << std::left << std::setw(width) << std::setfill('-') << "" << std::endl;

      // Data
      int rowCounter = 0;
      for (int rowIndex = 0; rowIndex < table_->num_rows(); ++rowIndex) {

        if (rowCounter >= options.getMaxNumRows())
          break;

        for (int columnIndex = 0; columnIndex < table_->num_columns(); ++columnIndex) {
          auto column = this->getColumnByIndex(columnIndex).value();
          auto value = column->element(rowIndex).value();
          ss << std::left << std::setw(columnWidth) << std::setfill(' ');
          ss << "| " + value->toString();
        }
        ss << std::endl;

        rowCounter++;
      }

      // Shape
      ss << std::left << std::setw(width) << std::setfill('-') << "" << std::endl;
      ss << table_->num_columns() << " cols x " << table_->num_rows() << " rows";
      if (rowCounter < table_->num_rows()) {
        ss << " (showing only top " << rowCounter << " rows)";
      }

      ss << std::endl;

      return ss.str();
    }
  }
}

std::string TupleSet::toString() const {
  if(valid())
    return fmt::format("<TupleSet2: {} x {}>", numColumns(), numRows());
  else
    return fmt::format("<TupleSet2: empty>", numColumns(), numRows());
}

std::shared_ptr<arrow::Scalar> TupleSet::visit(const std::function<std::shared_ptr<arrow::Scalar>(
        std::shared_ptr<arrow::Scalar>, arrow::RecordBatch &)> &fn) {

  arrow::Status arrowStatus;

  std::shared_ptr<arrow::RecordBatch> batch;
  arrow::TableBatchReader reader(*table_);
//  reader.set_chunksize(tuple::DefaultChunkSize);
  arrowStatus = reader.ReadNext(&batch);

  std::shared_ptr<arrow::Scalar> result;
  while (arrowStatus.ok() && batch) {
    result = fn(result, *batch);
    arrowStatus = reader.ReadNext(&batch);
  }

  return result;
}

std::vector<std::shared_ptr<arrow::Table>>
TupleSet::tupleSetVectorToArrowTableVector(const std::vector<std::shared_ptr<TupleSet>> &tupleSets) {
  std::vector<std::shared_ptr<arrow::Table>> tableVector;
  tableVector.reserve(tupleSets.size());
  for (const auto &tupleSet: tupleSets) {
    if (tupleSet->valid())
      tableVector.push_back(tupleSet->table_);
  }
  return tableVector;
}
