//
// Created by matt on 15/4/20.
//

#ifndef FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3PARTITION_H
#define FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3PARTITION_H

#include <fpdb/catalogue/Partition.h>
#include <fpdb/catalogue/Table.h>
#include <fpdb/caf/CAFUtil.h>
#include <string>
#include <memory>

using namespace std;

namespace fpdb::catalogue::s3 {

class S3Partition: public Partition {
public:
  explicit S3Partition(string s3Bucket,
                       string s3Object);

  explicit S3Partition(string s3Bucket,
                       string s3Object,
                       long numBytes);

  S3Partition() = default;
  S3Partition(const S3Partition&) = default;
  S3Partition& operator=(const S3Partition&) = default;

  [[nodiscard]] const string &getBucket() const;
  [[nodiscard]] const string &getObject() const;

  string toString() override;
  size_t hash() override;
  bool equalTo(shared_ptr<Partition> other) override;
  CatalogueEntryType getCatalogueEntryType() override;
  long toRawPartition() override;

  bool operator==(const S3Partition& other);
  bool operator==(long rawPartition);

private:
  string s3Bucket_;
  string s3Object_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, S3Partition& partition) {
    return f.object(partition).fields(f.field("numBytes", partition.numBytes_),
                                      f.field("zoneMap", partition.zoneMap_),
                                      f.field("bucket", partition.s3Bucket_),
                                      f.field("object", partition.s3Object_));
  }
};

}

using S3PartitionPtr = std::shared_ptr<fpdb::catalogue::s3::S3Partition>;

namespace caf {
template <>
struct inspector_access<S3PartitionPtr> : variant_inspector_access<S3PartitionPtr> {
  // nop
};
} // namespace caf

#endif //FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_S3_S3PARTITION_H
