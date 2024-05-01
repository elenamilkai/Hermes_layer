//
// Created by Elena Milkai on 10/14/21.
//

#ifndef FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHEKEY_H
#define FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHEKEY_H

#include <fpdb/catalogue/Partition.h>
#include <fpdb/catalogue/serialization/PartitionSerializer.h>
#include <fpdb/caf/CAFUtil.h>
#include <string>
#include <memory>

using namespace fpdb::catalogue;

namespace fpdb::delta {

class DeltaCacheKey {

public:
  DeltaCacheKey(long partition);

  DeltaCacheKey() = default;
  DeltaCacheKey(const DeltaCacheKey&) = default;
  DeltaCacheKey& operator=(const DeltaCacheKey&) = default;

  bool operator==(const DeltaCacheKey& other) const;
  bool operator!=(const DeltaCacheKey& other) const;

  static std::shared_ptr<DeltaCacheKey> make(long partition);

  [[nodiscard]] long getPartitionNumber() const;

  size_t hash();

  std::string toString();

private:
  long partition_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, DeltaCacheKey& key) {
    return f.object(key).fields(f.field("partition", key.partition_));
  };
};

struct DeltaKeyPointerHash {
  inline size_t operator()(const std::shared_ptr<DeltaCacheKey> &key) const {
    return key->hash();
  }
};

struct DeltaKeyPointerPredicate {
  inline bool operator()(const std::shared_ptr<DeltaCacheKey>& lhs, const std::shared_ptr<DeltaCacheKey>& rhs) const {
    return *lhs == *rhs;
  }
};

}

using DeltaCacheKeyPtr = std::shared_ptr<fpdb::delta::DeltaCacheKey>;

CAF_BEGIN_TYPE_ID_BLOCK(DeltaCacheKey, fpdb::caf::CAFUtil::DeltaCacheKey_first_custom_type_id)
CAF_ADD_TYPE_ID(DeltaCacheKey, (fpdb::delta::DeltaCacheKey))
CAF_END_TYPE_ID_BLOCK(DeltaCacheKey)

namespace caf {
template <>
struct inspector_access<DeltaCacheKeyPtr> : variant_inspector_access<DeltaCacheKeyPtr> {
  // nop
};
} // namespace caf

#endif //FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHEKEY_H
