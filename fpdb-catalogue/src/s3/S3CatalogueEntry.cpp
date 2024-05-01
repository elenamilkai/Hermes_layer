//
// Created by matt on 27/3/20.
//

#include <fpdb/catalogue/s3/S3CatalogueEntry.h>
#include <fpdb/catalogue/CatalogueEntryType.h>
#include <fmt/format.h>

#include <utility>
#include <spdlog/spdlog.h>

using namespace fpdb::catalogue;

namespace fpdb::catalogue::s3 {

S3CatalogueEntry::S3CatalogueEntry(string schemaName,
                                   string s3Bucket,
                                   shared_ptr<Catalogue> catalogue) :
  CatalogueEntry(S3,
                 move(schemaName),
                 move(catalogue)),
  s3Bucket_(move(s3Bucket)) {}

const string &S3CatalogueEntry::getS3Bucket() const {
  return s3Bucket_;
}

const shared_ptr<S3Table> &S3CatalogueEntry::getS3Table(const string& tableName) const {
  const auto &it = s3Tables_.find(tableName);
  if (it == s3Tables_.end()) {
    throw runtime_error(fmt::format("S3 table not found: {}", tableName));
  }
  return it->second;
}

vector<shared_ptr<Table>> S3CatalogueEntry::getTables() const {
  std::vector<shared_ptr<Table>> allTables(0);

  for (const auto& table_pair : s3Tables_) {
    allTables.emplace_back(table_pair.second);
  }
  return allTables;
}

string S3CatalogueEntry::getTypeName() const {
  return "S3CatalogueEntry";
}

void S3CatalogueEntry::addS3Table(const shared_ptr<S3Table> &s3Table) {
  s3Tables_.emplace(s3Table->getName(), s3Table);
}

string S3CatalogueEntry::getName() const {
  return fmt::format("s3://{}/{}", s3Bucket_, getSchemaName());
}

}
