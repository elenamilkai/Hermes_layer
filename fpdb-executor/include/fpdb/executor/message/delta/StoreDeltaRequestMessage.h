//
// Created by Elena Milkai on 10/14/21.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_STOREDELTAREQUESTMESSAGE_H_
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_STOREDELTAREQUESTMESSAGE_H_

#include <fpdb/executor/message/Message.h>
#include <fpdb/delta/DeltaCacheKey.h>
#include <fpdb/caf/CAFUtil.h>
#include <memory>

using namespace fpdb::delta;

namespace fpdb::executor::message {

class StoreDeltaRequestMessage : public Message {

public:
  StoreDeltaRequestMessage(std::shared_ptr<Table> table, std::shared_ptr<DeltaCacheKey> tailKey, std::string sender);

  StoreDeltaRequestMessage() = default;
  StoreDeltaRequestMessage(const StoreDeltaRequestMessage&) = default;
  StoreDeltaRequestMessage& operator=(const StoreDeltaRequestMessage&) = default;

  static std::shared_ptr<StoreDeltaRequestMessage> make(std::shared_ptr<Table> table, std::shared_ptr<DeltaCacheKey> tailKey, std::string sender);

  [[nodiscard]] std::string getTypeString() const override;

  [[nodiscard]] std::shared_ptr<Table> getTable() const;
  [[nodiscard]] std::shared_ptr<DeltaCacheKey> getTailKey() const;

  [[nodiscard]] std::string toString() const;

private:
  std::shared_ptr<Table> table_;
  std::shared_ptr<DeltaCacheKey> tailKey_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, StoreDeltaRequestMessage& msg) {
    return f.object(msg).fields(f.field("type", msg.type_),
                                f.field("sender", msg.sender_),
                                f.field("table", msg.table_),
                                f.field("tailKey", msg.tailKey_));
  };
};

}

using StoreDeltaRequestMessagePtr = std::shared_ptr<fpdb::executor::message::StoreDeltaRequestMessage>;

namespace caf {
template <>
struct inspector_access<StoreDeltaRequestMessagePtr> : variant_inspector_access<StoreDeltaRequestMessagePtr> {
  // nop
};
}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_STOREDELTAREQUESTMESSAGE_H_
