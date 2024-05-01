//
// Created by Elena Milkai on 10/14/21.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_LOADDELTARESPONSEMESSAGE_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_LOADDELTARESPONSEMESSAGE_H

#include <fpdb/executor/message/Message.h>
#include <fpdb/delta/DeltaCacheKey.h>
#include <fpdb/delta/DeltaCacheData.h>
#include <fpdb/delta/KeepMapCacheData.h>
#include <fpdb/caf/CAFUtil.h>
#include <memory>

using namespace fpdb::delta;

namespace fpdb::executor::message {

class LoadDeltaResponseMessage : public Message {

public:
  LoadDeltaResponseMessage(std::shared_ptr<Table> table,
                           std::vector<std::shared_ptr<DeltaCacheData>> deltas,
                           std::shared_ptr<std::vector<uint8_t>> stableBm,
                           std::string sender);

  LoadDeltaResponseMessage() = default;
  LoadDeltaResponseMessage(const LoadDeltaResponseMessage&) = default;
  LoadDeltaResponseMessage& operator=(const LoadDeltaResponseMessage&) = default;

  static std::shared_ptr<LoadDeltaResponseMessage> make(std::shared_ptr<Table> table,
                                                        std::vector<std::shared_ptr<DeltaCacheData>> deltas,
                                                        std::shared_ptr<std::vector<uint8_t>> stableBm,
                                                        std::string sender);

  [[nodiscard]] std::string getTypeString() const override;

  [[nodiscard]] std::shared_ptr<Table> getTable() const;
  [[nodiscard]] std::vector<std::shared_ptr<DeltaCacheData>> getDeltas() const;
  [[nodiscard]] std::shared_ptr<std::vector<uint8_t>> getStableBm() const;
  [[nodiscard]] std::string toString() const;

private:
  std::shared_ptr<Table> table_;
  std::vector<std::shared_ptr<DeltaCacheData>> deltas_;
  std::shared_ptr<std::vector<uint8_t>> stableBm_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, LoadDeltaResponseMessage& msg) {
    return f.object(msg).fields(f.field("type", msg.type_),
                                f.field("sender", msg.sender_),
                                f.field("table", msg.table_),
                                f.field("deltas", msg.deltas_),
                                f.field("stableBm", msg.stableBm_));
  };
  template <class Inspector> friend bool inspect(Inspector &f, std::shared_ptr<LoadDeltaResponseMessage> &x) {
    return f.object(x).fields(
        f.field("ptr", x));
  };
};

}

using LoadDeltaResponseMessagePtr = std::shared_ptr<fpdb::executor::message::LoadDeltaResponseMessage>;
using KeepMapMap = std::shared_ptr<std::map<std::pair<int, int>, int>>;
using StableBmMapMap = std::shared_ptr<std::vector<uint8_t>>;

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(KeepMapMap);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(StableBmMapMap);

namespace caf {
template <>
struct inspector_access<LoadDeltaResponseMessagePtr> : variant_inspector_access<LoadDeltaResponseMessagePtr> {
  // nop
};
} // namespace caf

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_LOADDELTARESPONSEMESSAGE_H