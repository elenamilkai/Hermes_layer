//
// Created by Yifei Yang on 11/8/21.
//

#include <fpdb/catalogue/Table.h>
#include <fmt/format.h>
#include <utility>

namespace fpdb::catalogue {

Table::Table(string name,
             const vector<string> &primaryKeyNames,
             const shared_ptr<arrow::Schema>& schema,
             const shared_ptr<format::Format>& format,
             const unordered_map<string, int> &apxColumnLengthMap,
             int apxRowLength,
             const unordered_set<string> &zonemapColumnNames) :
  name_(std::move(name)),
  primaryKeyNames_(primaryKeyNames),
  schema_(schema),
  format_(format),
  apxColumnLengthMap_(apxColumnLengthMap),
  apxRowLength_(apxRowLength),
  zonemapColumnNames_(zonemapColumnNames) {
  // manually construct the delta schema from the original table schema
  std::vector<std::shared_ptr<arrow::Field>> fields;
  for (const auto& field : schema_->fields()) {
    fields.push_back(field);
  }
  //shared_ptr<arrow::Field> timestampField = make_shared<arrow::Field>("timestamp", arrow::int64());
  shared_ptr<arrow::Field> typeField = make_shared<arrow::Field>("type", arrow::utf8());
  fields.push_back(typeField);
  //fields.push_back(timestampField);
  deltaSchema_ = ::arrow::schema(fields);
}

const string &Table::getName() const {
  return name_;
}

const shared_ptr<arrow::Schema> &Table::getSchema() const {
  return schema_;
}

const shared_ptr<arrow::Schema> &Table::getDeltaSchema() const {
  return deltaSchema_;
}

const shared_ptr<format::Format> &Table::getFormat() const {
  return format_;
}

vector<string> Table::getColumnNames() const {
  return schema_->field_names();
}

int Table::getApxColumnLength(const string &columnName) const {
  const auto &it = apxColumnLengthMap_.find(columnName);
  if (it == apxColumnLengthMap_.end()) {
    throw runtime_error(fmt::format("Column {} not found in Table {}.", columnName, name_));
  }
  return it->second;
}

int Table::getApxRowLength() const {
  return apxRowLength_;
}

const vector<string> Table::getPrimaryKeyColumnNames() const {
  return primaryKeyNames_;
}

}
