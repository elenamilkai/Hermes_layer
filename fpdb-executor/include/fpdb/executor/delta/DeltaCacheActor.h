//
// Created by Elena Milkai on 10/14/21.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_DELTA_DELTACACHEACTOR_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_DELTA_DELTACACHEACTOR_H

#include <fpdb/executor/message/delta/LoadDeltaRequestMessage.h>
#include <fpdb/executor/message/delta/LoadDeltaResponseMessage.h>
#include <fpdb/executor/message/delta/StoreDeltaRequestMessage.h>
#include <fpdb/delta/DeltaCache.h>
#include <fpdb/delta/KeepMapCache.h>
#include <fpdb/delta/StableBmCache.h>
#include <fpdb/executor/serialization/MessageSerializer.h>
#include <fpdb/caf/CAFUtil.h>
#include "../../../../../fpdb-logparser/cpp/include/deltapump/BinlogParser.h"
#include <string>
#include <unordered_map>

using namespace fpdb::executor::message;
using namespace fpdb::delta;
using namespace caf;
using namespace fpdb::logparser;

CAF_BEGIN_TYPE_ID_BLOCK(DeltaCacheActor, fpdb::caf::CAFUtil::DeltaCacheActor_first_custom_type_id)
CAF_ADD_ATOM(DeltaCacheActor, LoadDeltaAtom)
CAF_ADD_ATOM(DeltaCacheActor, StoreDeltaAtom)
CAF_ADD_ATOM(DeltaCacheActor, CacheDeltaAtom)
CAF_ADD_ATOM(DeltaCacheActor, MergedDeltaAtom)

CAF_END_TYPE_ID_BLOCK(DeltaCacheActor)

namespace fpdb::executor::delta {

struct DeltaCacheActorState {
  std::string name = "delta-cache";
  // Table Name - DeltaCache for that Table
  std::unordered_map<std::shared_ptr<Table>, std::shared_ptr<DeltaCache>> cache;
  std::unordered_map<std::shared_ptr<Table>, std::shared_ptr<StableBmCache>> stableBmCache;
  int queryId = 2022;
  long lastTimestamp = 0;

};

class DeltaCacheActor {

public:
  [[maybe_unused]] static behavior makeBehaviour(stateful_actor<DeltaCacheActorState> *self, const std::vector<std::shared_ptr<Table>>& tables);

  /**
   * Request for deltas coming from the DeltaMerge operators. This will also trigger a call from DeltaPump
   * as an instant tail update, triggered by a physical operator in foreground merge
   * @param msg
   * @param self
   * @return
   */
  static std::shared_ptr<LoadDeltaResponseMessage> loadMemoryDelta(const LoadDeltaRequestMessage& msg,
                                                                    stateful_actor<DeltaCacheActorState> *self);
  /**
   * Call DeltaPump to pump the deltas into cache everytime foreground merging is requesting a delta
   * @param self
   */
  static void deltaPump(stateful_actor<DeltaCacheActorState> *self);

  /**
   * Save in the delta cache the merged delta and remove the previous deltas
   * @param msg
   * @param self
   */
  static void storeMergedDelta(const MergedDeltaMessage& msg,
                               stateful_actor<DeltaCacheActorState> *self);

  /**
   * Merge old deltas and new deltas obtained when PeriodicGetTail os triggered.
   * Save merged deltas in-memory (for each table and partition).
   * @param self
   */
  static void mergeDeltasPeriodically(stateful_actor<DeltaCacheActorState> *self);
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_DELTA_DELTACACHEACTOR_H
