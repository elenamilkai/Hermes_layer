//
// Created by matt on 12/8/20.
//

#include "fpdb/tuple/parquet/ParquetReader.h"
#include <filesystem>

using namespace fpdb::tuple;

ParquetReader::ParquetReader(std::string Path) :
  FileReader(FileType::Parquet),
  path_(std::move(Path)) {}

tl::expected<std::shared_ptr<ParquetReader>, std::string> ParquetReader::make(const std::string &Path) {
  return std::make_shared<ParquetReader>(Path);
}

tl::expected<std::shared_ptr<TupleSet>, std::string>
ParquetReader::read(const std::vector<std::string> &columnNames, int64_t startPos, int64_t finishPos) {
  // open
  ::arrow::Status status;
  auto absolutePath = std::filesystem::absolute(path_);

  auto expectedInputStream = ::arrow::io::ReadableFile::Open(absolutePath);
  if (!expectedInputStream.ok())
    return tl::make_unexpected(expectedInputStream.status().message());
  auto inputStream = *expectedInputStream;

  std::unique_ptr<::parquet::arrow::FileReader> arrowReader;
  status = ::parquet::arrow::OpenFile(inputStream, ::arrow::default_memory_pool(), &arrowReader);
  if (!status.ok()) {
    close(inputStream);
    return tl::make_unexpected(status.message());
  }

  // read
  auto metadata = arrowReader->parquet_reader()->metadata();
  
  std::vector<int> rowGroupIndexes;
  for (int rowGroupIndex = 0; rowGroupIndex < metadata->num_row_groups(); ++rowGroupIndex) {
    auto rowGroupMetaData = metadata->RowGroup(rowGroupIndex);
    auto columnChunkMetaData = rowGroupMetaData->ColumnChunk(0);
    auto offset = columnChunkMetaData->file_offset();
  
    if (offset >= startPos && offset <= finishPos) {
      rowGroupIndexes.emplace_back(rowGroupIndex);
    }
  }

  std::unordered_map<std::string, bool> columnIndexMap;
  for (const auto &columnName: columnNames) {
	  columnIndexMap.emplace(ColumnName::canonicalize(columnName), true);
  }

  std::vector<int> columnIndexes;
  for (int columnIndex = 0; columnIndex < metadata->schema()->num_columns(); ++columnIndex) {
    auto columnMetaData = metadata->schema()->Column(columnIndex);
    if (columnIndexMap.find(ColumnName::canonicalize(columnMetaData->name())) != columnIndexMap.end()) {
      columnIndexes.emplace_back(columnIndex);
    }
  }

  if (columnIndexes.empty())
    SPDLOG_WARN(
      "ParquetReader will not read any data. The supplied column names did not match any columns in the file's schema. \n"
      "Parquet File: '{}'\n"
      "Columns: '{}'\n",
      path_,
      fmt::join(columnNames, ","));

  std::unique_ptr<::arrow::RecordBatchReader> recordBatchReader;
  status = arrowReader->GetRecordBatchReader(rowGroupIndexes, columnIndexes, &recordBatchReader);
  if (!status.ok()) {
    close(inputStream);
    return tl::make_unexpected(status.message());
  }

  std::shared_ptr<::arrow::Table> table;
  status = recordBatchReader->ReadAll(&table);
  if (!status.ok()) {
    close(inputStream);
    return tl::make_unexpected(status.message());
  }

  auto tableColumnNames = table->schema()->field_names();
  std::vector<std::string> canonicalColumnNames;
  std::transform(tableColumnNames.begin(), tableColumnNames.end(),
				 std::back_inserter(canonicalColumnNames),
				 [](auto name) -> auto { return ColumnName::canonicalize(name); });

  auto expectedTable = table->RenameColumns(canonicalColumnNames);
  if (expectedTable.ok())
	  table = *expectedTable;
  else {
    close(inputStream);
    return tl::make_unexpected(expectedTable.status().message());
  }
  auto tupleSet = TupleSet::make(table);

  // close and return
  close(inputStream);
  return tupleSet;
}

tl::expected<void, std::string> ParquetReader::close(const std::shared_ptr<arrow::io::ReadableFile> &inputStream) {
  if (inputStream && !inputStream->closed()) {
    auto result = inputStream->Close();
    if (!result.ok())
      return tl::make_unexpected(result.message());
  }
  return {};
}
