//
// Created by Elena Milkai on 8/2/22.
//

#ifndef FPDB_KEEPMAPCACHEKEY_H
#define FPDB_KEEPMAPCACHEKEY_H

#include <fpdb/catalogue/Partition.h>
#include <fpdb/catalogue/serialization/PartitionSerializer.h>
#include <fpdb/caf/CAFUtil.h>
#include <string>
#include <memory>

using namespace fpdb::catalogue;

namespace fpdb::delta {

class KeepMapCacheKey {

public:
  KeepMapCacheKey(long partition);

  KeepMapCacheKey() = default;
  KeepMapCacheKey(const KeepMapCacheKey&) = default;
  KeepMapCacheKey& operator=(const KeepMapCacheKey&) = default;

  bool operator==(const KeepMapCacheKey& other) const;
  bool operator!=(const KeepMapCacheKey& other) const;

  static std::shared_ptr<KeepMapCacheKey> make(long partition);

  [[nodiscard]] long getPartitionNumber() const;

  size_t hash();

  std::string toString();

private:
  long partition_;

  // caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, KeepMapCacheKey& key) {
    return f.object(key).fields(f.field("partition", key.partition_));
  };
};

struct KeepMapKeyPointerHash {
  inline size_t operator()(const std::shared_ptr<KeepMapCacheKey> &key) const {
    return key->hash();
  }
};

struct KeepMapKeyPointerPredicate {
  inline bool operator()(const std::shared_ptr<KeepMapCacheKey>& lhs, const std::shared_ptr<KeepMapCacheKey>& rhs) const {
    return *lhs == *rhs;
  }
};

}

using KeepMapCacheKeyPtr = std::shared_ptr<fpdb::delta::KeepMapCacheKey>;

CAF_BEGIN_TYPE_ID_BLOCK(KeepMapCacheKey, fpdb::caf::CAFUtil::KeepMapCacheKey_first_custom_type_id)
CAF_ADD_TYPE_ID(KeepMapCacheKey, (fpdb::delta::KeepMapCacheKey))
CAF_END_TYPE_ID_BLOCK(KeepMapCacheKey)

namespace caf {
template <>
struct inspector_access<KeepMapCacheKeyPtr> : variant_inspector_access<KeepMapCacheKeyPtr> {
  // nop
};
} // namespace caf

#endif//FPDB_KEEPMAPCACHEKEY_H
