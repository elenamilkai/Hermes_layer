//
// Created by Elena Milkai on 10/14/21.
//

#ifndef FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHEDATA_H
#define FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHEDATA_H

#include <memory>
#include <fpdb/tuple/TupleSet.h>

using namespace fpdb::tuple;

namespace fpdb::delta {

class DeltaCacheData {

public:
  DeltaCacheData(std::shared_ptr<TupleSet> delta, long timestamp);

  DeltaCacheData() = default;
  DeltaCacheData(const DeltaCacheData&) = default;
  DeltaCacheData& operator=(const DeltaCacheData&) = default;

  static std::shared_ptr<DeltaCacheData> make(std::shared_ptr<TupleSet> delta, long timestamp);

  [[nodiscard]] const std::shared_ptr<TupleSet> getDelta() const;
  [[nodiscard]] long getTimestamp() const;

private:
  std::shared_ptr<TupleSet> delta_;
  long timestamp_; // long type to prevent integer overflow

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, DeltaCacheData& data) {
    return f.object(data).fields(f.field("delta", data.delta_),
                                f.field("timestamp", data.timestamp_));
  };
};

}

using DeltaCacheDataPtr = std::shared_ptr<fpdb::delta::DeltaCacheData>;

CAF_BEGIN_TYPE_ID_BLOCK(DeltaCacheData, fpdb::caf::CAFUtil::DeltaCacheData_first_custom_type_id)
CAF_ADD_TYPE_ID(DeltaCacheData, (fpdb::delta::DeltaCacheData))
CAF_END_TYPE_ID_BLOCK(DeltaCacheData)

namespace caf {
template <>
struct inspector_access<DeltaCacheDataPtr> : variant_inspector_access<DeltaCacheDataPtr> {
  // nop
};
} // namespace caf

#endif //FPDB_FPDB_DELTA_INCLUDE_FPDB_DELTA_DELTACACHEDATA_H
