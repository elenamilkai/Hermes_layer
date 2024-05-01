//
// Created by Elena Milkai on 8/2/22.
//

#ifndef FPDB_KEEPMAPCACHEDATA_H
#define FPDB_KEEPMAPCACHEDATA_H

#include <memory>
#include <vector>
#include <fpdb/tuple/TupleSet.h>

using namespace fpdb::tuple;
namespace fpdb::delta {

class KeepMapCacheData {

public:
  explicit KeepMapCacheData(std::shared_ptr<std::map<std::pair<int, int>, int>> keepMap);
  KeepMapCacheData() = default;
  KeepMapCacheData(const KeepMapCacheData&) = default;
  KeepMapCacheData& operator=(const KeepMapCacheData&) = default;

  static std::shared_ptr<KeepMapCacheData> make(const std::shared_ptr<std::map<std::pair<int, int>, int>>& keepMap);

  [[nodiscard]] std::shared_ptr<std::map<std::pair<int, int>, int>> getKeepMap() const;

private:
  std::shared_ptr<std::map<std::pair<int, int>, int>> keepMap_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, KeepMapCacheData& data) {
    return f.object(data).fields(f.field("delta", data.keepMap_));
  };
};

}

using KeepMapCacheDataPtr = std::shared_ptr<fpdb::delta::KeepMapCacheData>;

CAF_BEGIN_TYPE_ID_BLOCK(KeepMapCacheData, fpdb::caf::CAFUtil::KeepMapCacheData_first_custom_type_id)
CAF_ADD_TYPE_ID(KeepMapCacheData, (fpdb::delta::KeepMapCacheData))
CAF_END_TYPE_ID_BLOCK(KeepMapCacheData)

namespace caf {
template <>
struct inspector_access<KeepMapCacheDataPtr> : variant_inspector_access<KeepMapCacheDataPtr> {
  // nop
};
} // namespace caf

#endif//FPDB_KEEPMAPCACHEDATA_H
