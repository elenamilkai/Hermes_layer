//
// Created by Elena Milkai on 2/20/23.
//

#ifndef FPDB_STABLEBMCACHEKEY_H
#define FPDB_STABLEBMCACHEKEY_H

#include <fpdb/catalogue/Partition.h>
#include <fpdb/catalogue/serialization/PartitionSerializer.h>
#include <fpdb/caf/CAFUtil.h>
#include <string>
#include <memory>

using namespace fpdb::catalogue;

namespace fpdb::delta {

class StableBmCacheKey {

public:
  StableBmCacheKey(long partition);

  StableBmCacheKey() = default;
  StableBmCacheKey(const StableBmCacheKey&) = default;
  StableBmCacheKey& operator=(const StableBmCacheKey&) = default;

  bool operator==(const StableBmCacheKey& other) const;
  bool operator!=(const StableBmCacheKey& other) const;

  static std::shared_ptr<StableBmCacheKey> make(long partition);

  [[nodiscard]] long getPartitionNumber() const;

  size_t hash();

  std::string toString();

private:
  long partition_;

  // caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, StableBmCacheKey& key) {
    return f.object(key).fields(f.field("partition", key.partition_));
  };
};

struct StableBmKeyPointerHash {
  inline size_t operator()(const std::shared_ptr<StableBmCacheKey> &key) const {
    return key->hash();
  }
};

struct StableBmKeyPointerPredicate {
  inline bool operator()(const std::shared_ptr<StableBmCacheKey>& lhs, const std::shared_ptr<StableBmCacheKey>& rhs) const {
    return *lhs == *rhs;
  }
};

}

using StableBmCacheKeyPtr = std::shared_ptr<fpdb::delta::StableBmCacheKey>;

CAF_BEGIN_TYPE_ID_BLOCK(StableBmCacheKey, fpdb::caf::CAFUtil::StableBmCacheKey_first_custom_type_id)
CAF_ADD_TYPE_ID(StableBmCacheKey, (fpdb::delta::StableBmCacheKey))
CAF_END_TYPE_ID_BLOCK(StableBmCacheKey)

namespace caf {
template <>
struct inspector_access<StableBmCacheKeyPtr> : variant_inspector_access<StableBmCacheKeyPtr> {
  // nop
};
} // namespace caf

#endif//FPDB_STABLEBMCACHEKEY_H
