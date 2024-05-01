//
// Created by Elena Milkai on 10/14/21.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_LOADDELTAREQUESTMESSAGE_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_LOADDELTAREQUESTMESSAGE_H

#include <fpdb/executor/message/Message.h>
#include <fpdb/delta/DeltaCacheKey.h>
#include <fpdb/delta/DeltaCacheData.h>
#include <fpdb/caf/CAFUtil.h>
#include <memory>

using namespace fpdb::delta;

namespace fpdb::executor::message {

class LoadDeltaRequestMessage : public Message {

public:
  LoadDeltaRequestMessage(std::shared_ptr<Table> table, std::shared_ptr<DeltaCacheKey> deltaKey, std::string sender, int queryId);

  LoadDeltaRequestMessage() = default;
  LoadDeltaRequestMessage(const LoadDeltaRequestMessage&) = default;
  LoadDeltaRequestMessage& operator=(const LoadDeltaRequestMessage&) = default;

  static std::shared_ptr<LoadDeltaRequestMessage> make(std::shared_ptr<Table> table, std::shared_ptr<DeltaCacheKey> deltaKey, std::string sender, int queryId);

  [[nodiscard]] std::string getTypeString() const override;

  [[nodiscard]] std::shared_ptr<Table> getTable() const;
  [[nodiscard]] std::shared_ptr<DeltaCacheKey> getDeltaKey() const;
  [[nodiscard]] int getQueryId() const;

  [[nodiscard]] std::string toString() const;

private:
  std::shared_ptr<Table> table_;
  std::shared_ptr<DeltaCacheKey> deltaKey_;
  int queryId_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, LoadDeltaRequestMessage& msg) {
    return f.object(msg).fields(f.field("type", msg.type_),
                                f.field("sender", msg.sender_),
                                f.field("table", msg.table_),
                                f.field("deltaKey", msg.deltaKey_),
                                f.field("queryId", msg.queryId_));
  };
};

}

using LoadDeltaRequestMessagePtr = std::shared_ptr<fpdb::executor::message::LoadDeltaRequestMessage>;

namespace caf {
template <>
struct inspector_access<LoadDeltaRequestMessagePtr> : variant_inspector_access<LoadDeltaRequestMessagePtr> {
  // nop
};
} // namespace caf

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_MESSAGE_DELTA_LOADDELTAREQUESTMESSAGE_H
