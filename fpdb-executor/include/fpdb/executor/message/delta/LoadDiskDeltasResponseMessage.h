//
// Created by Elena Milkai on 3/8/22.
//

#ifndef FPDB_LOADDISKDELTASRESPONSEMESSAGE_H
#define FPDB_LOADDISKDELTASRESPONSEMESSAGE_H

#include <fpdb/executor/message/Message.h>
#include <fpdb/tuple/TupleSet.h>
#include <fpdb/caf/CAFUtil.h>
#include <memory>

using namespace fpdb::tuple;

namespace fpdb::executor::message {

    class LoadDiskDeltasResponseMessage : public Message {

    public:
        LoadDiskDeltasResponseMessage(std::shared_ptr<TupleSet> diskDeltasTuples, int timestamp, std::string sender);

        LoadDiskDeltasResponseMessage() = default;
        LoadDiskDeltasResponseMessage(const LoadDiskDeltasResponseMessage&) = default;
        LoadDiskDeltasResponseMessage& operator=(const LoadDiskDeltasResponseMessage&) = default;

        static std::shared_ptr<LoadDiskDeltasResponseMessage> make(std::shared_ptr<TupleSet> diskDeltasTuples,
                                                                   int timestamp,
                                                                   std::string sender);

        [[nodiscard]] std::string getTypeString() const override;

        [[nodiscard]] std::shared_ptr<TupleSet> getTuples() const;
        [[nodiscard]] int getTimestamp() const;

        [[nodiscard]] std::string toString() const;

    private:
        std::shared_ptr<TupleSet> diskDeltasTuples_;
        int timestamp_;

// caf inspect
    public:
        template <class Inspector>
        friend bool inspect(Inspector& f, LoadDiskDeltasResponseMessage& msg) {
            return f.object(msg).fields(f.field("type", msg.type_),
                                        f.field("sender", msg.sender_),
                                        f.field("diskDeltasTuples", msg.diskDeltasTuples_),
                                        f.field("timestamp", msg.timestamp_));
        };
    };

}

using LoadDiskDeltasResponseMessagePtr = std::shared_ptr<fpdb::executor::message::LoadDiskDeltasResponseMessage>;

namespace caf {
    template <>
    struct inspector_access<LoadDiskDeltasResponseMessagePtr> : variant_inspector_access<LoadDiskDeltasResponseMessagePtr> {
        // nop
    };
} // namespace caf

#endif //FPDB_LOADDISKDELTASRESPONSEMESSAGE_H
