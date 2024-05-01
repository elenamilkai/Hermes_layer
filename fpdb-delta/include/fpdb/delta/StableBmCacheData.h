//
// Created by Elena Milkai on 2/20/23.
//

#ifndef FPDB_STABLEBMCACHEDATA_H
#define FPDB_STABLEBMCACHEDATA_H
#include <memory>
#include <vector>
#include <fpdb/tuple/TupleSet.h>

using namespace fpdb::tuple;
namespace fpdb::delta {

class StableBmCacheData {

public:
  explicit StableBmCacheData(std::shared_ptr<std::vector<uint8_t>> stableBm);
  StableBmCacheData() = default;
  StableBmCacheData(const StableBmCacheData&) = default;
  StableBmCacheData& operator=(const StableBmCacheData&) = default;

  static std::shared_ptr<StableBmCacheData> make(const std::shared_ptr<std::vector<uint8_t>>& stableBm);

  [[nodiscard]] std::shared_ptr<std::vector<uint8_t>> getStableBm() const;

private:
  std::shared_ptr<std::vector<uint8_t>> stableBm_;

  // caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, StableBmCacheData& data) {
    return f.object(data).fields(f.field("delta", data.stableBm_));
  };
};

}

using StableBmCacheDataPtr = std::shared_ptr<fpdb::delta::StableBmCacheData>;

CAF_BEGIN_TYPE_ID_BLOCK(StableBmCacheData, fpdb::caf::CAFUtil::StableBmCacheData_first_custom_type_id)
CAF_ADD_TYPE_ID(StableBmCacheData, (fpdb::delta::StableBmCacheData))
CAF_END_TYPE_ID_BLOCK(StableBmCacheData)

namespace caf {
template <>
struct inspector_access<StableBmCacheDataPtr> : variant_inspector_access<StableBmCacheDataPtr> {
  // nop
};
} // namespace caf
#endif//FPDB_STABLEBMCACHEDATA_H
