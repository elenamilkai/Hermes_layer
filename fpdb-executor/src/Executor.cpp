//
// Created by Yifei Yang on 11/23/21.
//

#include <fpdb/executor/Executor.h>
#include <fpdb/executor/Execution.h>
#include <fpdb/executor/cache/SegmentCacheActor.h>
#include <fpdb/executor/delta/DeltaCacheActor.h>

using namespace fpdb::executor::cache;
using namespace fpdb::executor::delta;

namespace fpdb::executor {

Executor::Executor(const shared_ptr<::caf::actor_system> &actorSystem,
                   const vector<::caf::node_id> &nodes,
                   const shared_ptr<Mode> &mode,
                   const shared_ptr<CachingPolicy> &cachingPolicy,
                   const vector<shared_ptr<Table>> &tables,
                   bool showOpTimes,
                   bool showScanMetrics) :
  actorSystem_(actorSystem),
  nodes_(nodes),
  cachingPolicy_(cachingPolicy),
  mode_(mode),
  queryCounter_(0),
  running_(false),
  showOpTimes_(showOpTimes),
  showScanMetrics_(showScanMetrics),
  tables_(tables) {
  periodicGetTailHandler_ = make_unique<delta::PeriodicGetTailHandler>();
}

Executor::~Executor() {
  if (running_) {
    stop();
  }
}

void Executor::start() {
  rootActor_ = make_unique<::caf::scoped_actor>(*actorSystem_);
  if ((mode_->id() == CACHING_ONLY || mode_->id() == HYBRID) && !cachingPolicy_) {
    throw runtime_error(fmt::format("Failed to start, missing caching policy for mode: {}", mode_->toString()));
  }

  // initialize delta cache for HTAP
  // TODO: mode differentiation
//  SPDLOG_CRITICAL("initialize deltacacheActor");
  deltaCacheActor_ = actorSystem_->spawn(DeltaCacheActor::makeBehaviour, tables_);

  if (cachingPolicy_) {
    segmentCacheActor_ = actorSystem_->spawn(SegmentCacheActor::makeBehaviour, cachingPolicy_, mode_);
  } else {
    segmentCacheActor_ = nullptr;
  }
  // Start PeriodicGetTailHandler thread
  periodicGetTailHandler_->start(500000000, deltaCacheActor_, actorSystem_);
  running_ = true;
}

void Executor::stop() {
  // Stop the cache actor if cache is used
  periodicGetTailHandler_->stop();
  if (isCacheUsed()) {
    (*rootActor_)->send_exit(::caf::actor_cast<::caf::actor>(segmentCacheActor_), ::caf::exit_reason::user_shutdown);
  }

  // shutdown delta cache actor
  (*rootActor_)->send_exit(::caf::actor_cast<::caf::actor>(deltaCacheActor_), ::caf::exit_reason::user_shutdown);

  // Stop the root actor (seems, being defined by "scope", it needs to actually be destroyed to stop it)
  rootActor_.reset();

  this->actorSystem_->await_all_actors_done();
  running_ = false;
}

pair<shared_ptr<TupleSet>, long> Executor::execute(const shared_ptr<PhysicalPlan> &physicalPlan,
                                                   bool isDistributed) {
  const auto &execution = make_shared<Execution>(nextQueryId(),
                                                 actorSystem_,
                                                 nodes_,
                                                 segmentCacheActor_,
                                                 deltaCacheActor_,
                                                 physicalPlan,
                                                 isDistributed);
  const auto &result = execution->execute();
  long elapsedTime = execution->getElapsedTime();
  if (showOpTimes_ || showScanMetrics_) {
    cout << execution->showMetrics(showOpTimes_, showScanMetrics_) << endl;
  }
  return make_pair(result, elapsedTime);
}

const actor &Executor::getSegmentCacheActor() const {
  return segmentCacheActor_;
}

const shared_ptr<::caf::actor_system> &Executor::getActorSystem() const {
  return actorSystem_;
}

const vector<shared_ptr<Table>> &Executor::getTables() const {
  return tables_;
}

const actor &Executor::getDeltaCacheActor() const {
  return deltaCacheActor_;
}

bool Executor::isCacheUsed() {
  return cachingPolicy_ != nullptr;
}

long Executor::nextQueryId() {
  return queryCounter_.fetch_add(1);
}

std::string Executor::showCacheMetrics() {
  size_t hitNum, missNum;
  size_t shardHitNum, shardMissNum;

  auto errorHandler = [&](const ::caf::error &error) {
    throw std::runtime_error(to_string(error));
  };

  if (!isCacheUsed()) {
    hitNum = 0;
    missNum = 0;
    shardHitNum = 0;
    shardMissNum = 0;
  } else {
    scoped_actor self{*actorSystem_};
    self->request(segmentCacheActor_, infinite, GetNumHitsAtom_v).receive(
            [&](size_t numHits) {
              hitNum = numHits;
            },
            errorHandler);

    self->request(segmentCacheActor_, infinite, GetNumMissesAtom_v).receive(
            [&](size_t numMisses) {
              missNum = numMisses;
            },
            errorHandler);

    self->request(segmentCacheActor_, infinite, GetNumShardHitsAtom_v).receive(
            [&](size_t numShardHits) {
              shardHitNum = numShardHits;
            },
            errorHandler);

    self->request(segmentCacheActor_, infinite, GetNumShardMissesAtom_v).receive(
            [&](size_t numShardMisses) {
              shardMissNum = numShardMisses;
            },
            errorHandler);
  }

  double hitRate = (hitNum + missNum == 0) ? 0.0 : (double) hitNum / (double) (hitNum + missNum);
  double shardHitRate = (shardHitNum + shardMissNum == 0) ? 0.0 : (double) shardHitNum / (double) (shardHitNum + shardMissNum);

  std::stringstream ss;
  ss << std::endl;

  ss << std::left << std::setw(60) << "Hit num:";
  ss << std::left << std::setw(40) << hitNum;
  ss << std::endl;

  ss << std::left << std::setw(60) << "Miss num:";
  ss << std::left << std::setw(40) << missNum;
  ss << std::endl;

  ss << std::left << std::setw(60) << "Hit rate:";
  ss << std::left << std::setw(40) << hitRate;
  ss << std::endl;

  ss << std::left << std::setw(60) << "Shard Hit num:";
  ss << std::left << std::setw(40) << shardHitNum;
  ss << std::endl;

  ss << std::left << std::setw(60) << "Shard Miss num:";
  ss << std::left << std::setw(40) << shardMissNum;
  ss << std::endl;

  ss << std::left << std::setw(60) << "Shard Hit rate:";
  ss << std::left << std::setw(40) << shardHitRate;
  ss << std::endl;

  ss << std::endl;

  return ss.str();
}

void Executor::clearCacheMetrics() {
  if (isCacheUsed()) {
    (*rootActor_)->anon_send(segmentCacheActor_, ClearMetricsAtom_v);
  }
}

double Executor::getCrtQueryHitRatio() {
  size_t crtQueryHitNum;
  size_t crtQueryMissNum;

  if (!isCacheUsed()) {
    crtQueryHitNum = 0;
    crtQueryMissNum = 0;
  } else {
    auto errorHandler = [&](const ::caf::error &error) {
      throw std::runtime_error(to_string(error));
    };

    // NOTE: Creating a new scoped_actor will work, but can use rootActor_ as well
    scoped_actor self{*actorSystem_};
    self->request(segmentCacheActor_, infinite, GetCrtQueryNumHitsAtom_v).receive(
            [&](size_t numHits) {
              crtQueryHitNum = numHits;
            },
            errorHandler);

    self->request(segmentCacheActor_, infinite, GetCrtQueryNumMissesAtom_v).receive(
            [&](size_t numMisses) {
              crtQueryMissNum = numMisses;
            },
            errorHandler);
  }

  return (crtQueryHitNum + crtQueryMissNum == 0) ? 0.0 :
    (double) crtQueryHitNum / (double) (crtQueryHitNum + crtQueryMissNum);
}

double Executor::getCrtQueryShardHitRatio() {
  size_t crtQueryShardHitNum;
  size_t crtQueryShardMissNum;

  if (isCacheUsed()) {
    crtQueryShardHitNum = 0;
    crtQueryShardMissNum = 0;
  } else {
    auto errorHandler = [&](const ::caf::error &error) {
      throw std::runtime_error(to_string(error));
    };

    // NOTE: Creating a new scoped_actor will work, but can use rootActor_ as well
    scoped_actor self{*actorSystem_};
    self->request(segmentCacheActor_, infinite, GetCrtQueryNumShardHitsAtom_v).receive(
            [&](size_t numShardHits) {
              crtQueryShardHitNum = numShardHits;
            },
            errorHandler);

    self->request(segmentCacheActor_, infinite, GetCrtQueryNumShardMissesAtom_v).receive(
            [&](size_t numShardMisses) {
              crtQueryShardMissNum = numShardMisses;
            },
            errorHandler);
  }

  return (crtQueryShardHitNum + crtQueryShardMissNum == 0) ? 0.0 :
    (double) crtQueryShardHitNum / (double) (crtQueryShardHitNum + crtQueryShardMissNum);
}

}
