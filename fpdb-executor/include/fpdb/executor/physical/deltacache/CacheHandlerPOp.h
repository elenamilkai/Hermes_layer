//
// Created by Elena Milkai on 3/24/22.
//
#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTACACHE_CACHEHANDLERPOP_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTACACHE_CACHEHANDLERPOP_H

#include <fpdb/executor/message/delta/LoadDeltaRequestMessage.h>
#include <fpdb/executor/message/delta/LoadDeltaResponseMessage.h>
#include <fpdb/executor/message/delta/StoreDeltaRequestMessage.h>
#include <fpdb/executor/message/delta/MergedDeltaMessage.h>
#include "fpdb/executor/message/MessageType.h"
#include "fpdb/catalogue/Partition.h"
#include "fpdb/executor/physical/PhysicalOp.h"
#include <fpdb/tuple/TupleSet.h>
#include <string>

using namespace fpdb::executor::message;

namespace fpdb::executor::physical::deltacache  {
    class CacheHandlerPOp : public PhysicalOp {
    public:
        explicit CacheHandlerPOp(std::string name,
                                 std::shared_ptr<fpdb::catalogue::Table> targetTable,
                                 std::shared_ptr<fpdb::catalogue::Partition> targetPartition,
                                 const std::vector<std::string> &projectColumnNames,
                                 int nodeId,
                                 std::string deltaMergeName);


        CacheHandlerPOp() = default;
        CacheHandlerPOp(const CacheHandlerPOp &) = default;
        CacheHandlerPOp &operator=(const CacheHandlerPOp &) = default;

        static std::shared_ptr<CacheHandlerPOp> make(std::string name,
                                                     std::shared_ptr<fpdb::catalogue::Table> targetTable,
                                                     std::shared_ptr<fpdb::catalogue::Partition> targetPartition,
                                                     const std::vector<std::string> &projectColumnNames,
                                                     int nodeId,
                                                     std::string deltaMergeName);

        void onReceive(const message::Envelope &msg) override;

        void onStart();
        void clear() override;
        string getTypeString() const override;

        /**
         * Function executed after CacheHandler receives a LoadDeltasRequestMessage message from DeltaMerge. It passes
         * the message to DeltaCacheActor.
         * @param message
         */
        void OnDeltasRequest(const LoadDeltaRequestMessage &message);

        /**
         * Function executed after CacheHandler receives a LoadMessageResponseMessage with requested deltas from the
         * DeltaCacheActor. Then CacheHandler passes the message to DeltaMerge and notifies all that is complete.
         * @param message
         */
        void OnReceiveResponse(const LoadDeltaResponseMessage &message);

        /**
         * Function executed after CacheHandler receives a DeltaMergedMessage message from DeltaMerge. It passes
         * the message to DeltaCacheActor.
         * @param message
         */
        void OnReceiveMergedDelta(const MergedDeltaMessage &message);

    private:
        std::shared_ptr<fpdb::catalogue::Table> targetTable_;
        std::shared_ptr<fpdb::catalogue::Partition> targetPartition_;
        std::string deltaMergeName_;

        // caf inspect
    public:
        template<class Inspector>
        friend bool inspect(Inspector& f, CacheHandlerPOp& op) {
            return f.object(op).fields(f.field("name", op.name_),
                                       f.field("type", op.type_),
                                       f.field("projectColumnNames", op.projectColumnNames_),
                                       f.field("nodeId", op.nodeId_),
                                       f.field("queryId", op.queryId_),
                                       f.field("opContext", op.opContext_),
                                       f.field("producers", op.producers_),
                                       f.field("consumers", op.consumers_),
                                       f.field("targetTable", op.targetTable_),
                                       f.field("targetTable", op.targetTable_),
                                       f.field("deltaMergeName", op.deltaMergeName_));
        }
    };
}

#endif // FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_DELTACACHE_CACHEHANDLERPOP_H

