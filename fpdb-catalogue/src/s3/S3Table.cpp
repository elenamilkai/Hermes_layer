//
// Created by Yifei Yang on 11/8/21.
//

#include <fpdb/catalogue/s3/S3Table.h>

using namespace std;

namespace fpdb::catalogue::s3 {

S3Table::S3Table(const string &name,
                 const vector<string> &primaryKeyNames,
                 const shared_ptr<arrow::Schema> &schema,
                 const shared_ptr<format::Format> &format,
                 const unordered_map<string, int> &apxColumnLengthMap,
                 int apxRowLength,
                 const unordered_set<string> &zonemapColumnNames,
                 const vector<shared_ptr<S3Partition>> &s3Partitions) :
  Table(name, primaryKeyNames, schema, format, apxColumnLengthMap, apxRowLength, zonemapColumnNames),
  s3Partitions_(s3Partitions) {}

const vector<shared_ptr<S3Partition>> &S3Table::getS3Partitions() const {
  return s3Partitions_;
}

CatalogueEntryType S3Table::getCatalogueEntryType() {
  return S3;
}

}
