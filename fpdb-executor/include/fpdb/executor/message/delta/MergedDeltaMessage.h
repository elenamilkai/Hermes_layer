//
// Created by Elena Milkai on 5/22/22.
//

#ifndef FPDB_MERGEDDELTAMESSAGE_H
#define FPDB_MERGEDDELTAMESSAGE_H

#include <fpdb/executor/message/Message.h>
#include <fpdb/tuple/TupleSet.h>
#include <fpdb/delta/DeltaCacheKey.h>
#include <fpdb/delta/DeltaCacheData.h>
#include <fpdb/caf/CAFUtil.h>
#include <memory>

using namespace fpdb::tuple;
using namespace fpdb::delta;

namespace fpdb::executor::message {

class MergedDeltaMessage : public Message {

public:
  MergedDeltaMessage(std::shared_ptr<Table> table,
                     long partition,
                     std::shared_ptr<TupleSet> mergedDeltaTuples,
                     std::shared_ptr<std::vector<uint8_t>> stableBm,
                     std::string sender);

  MergedDeltaMessage() = default;
  MergedDeltaMessage(const MergedDeltaMessage&) = default;
  MergedDeltaMessage& operator=(const MergedDeltaMessage&) = default;

  static std::shared_ptr<MergedDeltaMessage> make(std::shared_ptr<Table> table,
                                                  long partition,
                                                  std::shared_ptr<TupleSet> mergedDeltaTuples,
                                                  std::shared_ptr<std::vector<uint8_t>> stableBm,
                                                  const std::string& sender);

  [[nodiscard]] std::string getTypeString() const override;
  [[nodiscard]] std::shared_ptr<TupleSet> getMergedDelta() const;
  [[nodiscard]] std::shared_ptr<Table> getTable() const;
  [[nodiscard]] vector<std::pair<int, int>> getDeletedKeys() const;
  [[nodiscard]] std::shared_ptr<std::vector<uint8_t>> getStableBm() const;
  [[nodiscard]] long getPartition() const;
  [[nodiscard]] std::string toString() const;

private:
  std::shared_ptr<Table> table_;
  long partition_;
  std::shared_ptr<TupleSet> mergedDeltaTuples_;
  std::shared_ptr<std::vector<uint8_t>> stableBm_;

  // caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, MergedDeltaMessage& msg) {
    return f.object(msg).fields(f.field("type", msg.type_),
                                f.field("sender", msg.sender_),
                                f.field("table", msg.table_),
                                f.field("partition", msg.partition_),
                                f.field("stableBm", msg.stableBm_),
                                f.field("mergedDeltaTuples", msg.mergedDeltaTuples_));
  };
};
}

using MergedDeltaMessagePtr = std::shared_ptr<fpdb::executor::message::MergedDeltaMessage>;

namespace caf {
template <>
struct inspector_access<MergedDeltaMessagePtr> : variant_inspector_access<MergedDeltaMessagePtr> {
  // nop
};
} // namespace caf

#endif//FPDB_MERGEDDELTAMESSAGE_H