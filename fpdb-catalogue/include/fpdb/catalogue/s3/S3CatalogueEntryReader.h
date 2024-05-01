//
// Created by Yifei Yang on 11/9/21.
//

#ifndef FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3CATALOGUEENTRYREADER_H
#define FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3CATALOGUEENTRYREADER_H

#include <fpdb/catalogue/s3/S3CatalogueEntry.h>
#include <fpdb/catalogue/Catalogue.h>
#include <fpdb/catalogue/format/Format.h>
#include <fpdb/tuple/Scalar.h>
#include <nlohmann/json.hpp>
#include <aws/s3/S3Client.h>

using namespace fpdb::tuple;
using namespace Aws::S3;
using namespace std;
using json = nlohmann::json;

namespace fpdb::catalogue::s3 {

class S3CatalogueEntryReader {
public:
  static shared_ptr<S3CatalogueEntry> readS3CatalogueEntry(const shared_ptr<Catalogue> &catalogue,
                                                           const string &s3Bucket,
                                                           const string &schemaName,
                                                           const shared_ptr<S3Client> &s3Client);
  static void readPartitionSize(const shared_ptr<S3Client> &s3Client,
                              const string &s3Bucket,
                              const string &schemaName,
                              unordered_map<string, vector<shared_ptr<S3Partition>>> &s3PartitionsMap);

private:
  static void readSchema(const json &schemaJObj,
                         const string &s3Bucket,
                         const string &schemaName,
                         unordered_map<string, shared_ptr<arrow::Schema>> &schemaMap,
                         unordered_map<string, shared_ptr<format::Format>> &formatMap,
                         unordered_map<string, vector<string>> &primaryKeyNamesMap,
                         unordered_map<string, vector<shared_ptr<S3Partition>>> &s3PartitionsMap);
  static void readStats(const json &statsJObj,
                        unordered_map<string, unordered_map<string, int>> &apxColumnLengthMapMap,
                        unordered_map<string, int> &apxRowLengthMap);
  static void readZoneMap(const json &zonemapJObj,
                          const unordered_map<string, shared_ptr<arrow::Schema>> &schemaMap,
                          unordered_map<string, vector<shared_ptr<S3Partition>>> &s3PartitionsMap,
                          unordered_map<string, unordered_set<string>> &zonemapColumnNamesMap);
  /*static void readPartitionSize(const shared_ptr<S3Client> &s3Client,
                                const string &s3Bucket,
                                const string &schemaName,
                                unordered_map<string, vector<shared_ptr<S3Partition>>> &s3PartitionsMap);*/

  static shared_ptr<arrow::DataType> strToDataType(const string &str);
  static pair<shared_ptr<Scalar>, shared_ptr<Scalar>>
    jsonToMinMaxLiterals(const json &jObj, const shared_ptr<arrow::DataType> &datatype);
};

}


#endif //FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3CATALOGUEENTRYREADER_H
