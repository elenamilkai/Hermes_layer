//
// Created by matt on 27/3/20.
//

#ifndef FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3CATALOGUEENTRY_H
#define FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3CATALOGUEENTRY_H

#include <fpdb/catalogue/CatalogueEntry.h>
#include <fpdb/catalogue/Catalogue.h>
#include <fpdb/catalogue/s3/S3Table.h>

#include <string>

using namespace fpdb::catalogue;
using namespace std;

namespace fpdb::catalogue::s3 {

class S3CatalogueEntry : public CatalogueEntry {

public:
  S3CatalogueEntry(string schemaName,
                   string s3Bucket,
                   shared_ptr<Catalogue> catalogue);
  ~S3CatalogueEntry() override = default;

  const string &getS3Bucket() const;
  const shared_ptr<S3Table> &getS3Table(const string& tableName) const;
  // vector<shared_ptr<S3Table>> getAllS3Tables() const;
  string getTypeName() const override;
  string getName() const override;
  vector<shared_ptr<Table>> getTables() const override;

  void addS3Table(const shared_ptr<S3Table> &s3Table);

private:
  string s3Bucket_;
  unordered_map<string, shared_ptr<S3Table>> s3Tables_;
};

}

#endif //FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3CATALOGUEENTRY_H
