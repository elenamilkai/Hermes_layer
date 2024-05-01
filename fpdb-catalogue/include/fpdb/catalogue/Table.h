//
// Created by Yifei Yang on 11/8/21.
//

#ifndef FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_TABLE_H
#define FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_TABLE_H

#include <fpdb/catalogue/CatalogueEntry.h>
#include <fpdb/catalogue/format/Format.h>
#include <arrow/type.h>
#include <unordered_set>
#include <unordered_map>

using namespace std;

namespace fpdb::catalogue {

class CatalogueEntry;

class Table {
public:
  Table(string name,
        const vector<string> &primaryKeyNames,
        const shared_ptr<arrow::Schema>& schema,
        const shared_ptr<format::Format>& format,
        const unordered_map<string, int> &apxColumnLengthMap,
        int apxRowLength,
        const unordered_set<string> &zonemapColumnNames);
  Table() = default;
  Table(const Table&) = default;
  Table& operator=(const Table&) = default;
  virtual ~Table() = default;

  const string &getName() const;
  const vector<string> getPrimaryKeyColumnNames() const;
  const shared_ptr<arrow::Schema> &getSchema() const;
  const shared_ptr<arrow::Schema> &getDeltaSchema() const;
  const shared_ptr<format::Format> &getFormat() const;
  vector<string> getColumnNames() const;
  int getApxColumnLength(const string &columnName) const;
  int getApxRowLength() const;

  virtual CatalogueEntryType getCatalogueEntryType() = 0;

protected:
  string name_;
  vector<string> primaryKeyNames_;
  shared_ptr<arrow::Schema> schema_;
  shared_ptr<arrow::Schema> deltaSchema_;
  shared_ptr<format::Format> format_;
  unordered_map<string, int> apxColumnLengthMap_;   // apx: approximate
  int apxRowLength_;
  unordered_set<string> zonemapColumnNames_;
};

}


#endif //FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_TABLE_H
