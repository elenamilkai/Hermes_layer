//
// Created by matt on 4/6/20.
//

#include <fpdb/cache/policy/CachingPolicy.h>
#include <fpdb/cache/Util.h>
#include <fpdb/catalogue/s3/S3CatalogueEntry.h>
#include <fmt/format.h>
#include <utility>

namespace fpdb::cache::policy {

CachingPolicy::CachingPolicy(CachingPolicyType type,
                             size_t maxSize,
                             std::shared_ptr<CatalogueEntry> catalogueEntry,
                             bool readSegmentSize) :
  type_(type),
  maxSize_(maxSize),
  freeSize_(maxSize),
  catalogueEntry_(std::move(catalogueEntry)) {

  // read segment size if the caching policy needs
  if (readSegmentSize && catalogueEntry_) {
    if (catalogueEntry_->getType() == S3) {
      const auto schemaName = catalogueEntry_->getSchemaName();
      filesystem::path metadataPath = catalogueEntry_
              ->getCatalogue().lock()
              ->getMetadataPath()
              .append(schemaName)
              .append("segment_info");
      const auto &s3Bucket = std::static_pointer_cast<s3::S3CatalogueEntry>(catalogueEntry_)->getS3Bucket();
      segmentSizeMap_ = Util::readSegmentKeySize(s3Bucket, schemaName, metadataPath);
    } else {
      throw runtime_error(fmt::format("Read segment size unsupported, catalogue entry: {}",
                                      catalogueEntry->getName()));
    }
  }
}

size_t CachingPolicy::getFreeSize() const {
  return freeSize_;
}

CachingPolicyType CachingPolicy::getType() const {
  return type_;
}

size_t CachingPolicy::getSegmentSize(const shared_ptr<SegmentKey> &segmentKey) const {
  auto key = segmentSizeMap_.find(segmentKey);
  if (key != segmentSizeMap_.end()) {
    return segmentSizeMap_.at(segmentKey);
  }
  throw std::runtime_error("Segment key not found in getSegmentSize: " + segmentKey->toString());
}

const unordered_map<std::shared_ptr<SegmentKey>, size_t, SegmentKeyPointerHash, SegmentKeyPointerPredicate> &
CachingPolicy::getSegmentSizeMap() const {
  return segmentSizeMap_;
}

}
