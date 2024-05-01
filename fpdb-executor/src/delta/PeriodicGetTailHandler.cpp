//
// Created by xiaohans on 5/8/22.
//

#include <fpdb/executor/delta/PeriodicGetTailHandler.h>
#include <fpdb/executor/delta/DeltaCacheActor.h>

namespace fpdb::executor::delta {

PeriodicGetTailHandler::PeriodicGetTailHandler() :
        stop_flag(std::move(true)),
        periodicGetTailThread(NULL) {}

PeriodicGetTailHandler::~PeriodicGetTailHandler() {
    if (periodicGetTailThread != NULL) {
        delete periodicGetTailThread;
    }
}

void PeriodicGetTailHandler::start(int periodTimeMs, const ::caf::actor &deltaCacheActor,
                                   const shared_ptr<::caf::actor_system> &actorSystem) {
    if (periodicGetTailThread != NULL) {
        this->stop();
    }
    this->deltaCacheActor_ = deltaCacheActor;
    this->rootActor_ = make_unique<::caf::scoped_actor>(*actorSystem);
    this->stop_flag = false;
    this->periodTimeMs = periodTimeMs;
    this->periodicGetTailThread = new std::thread (&PeriodicGetTailHandler::startPeriodicGetTail, this);
}

void PeriodicGetTailHandler::stop() {
    this->stop_flag = true;
    if (this->periodicGetTailThread != NULL) {
        this->periodicGetTailThread->join();
        delete this->periodicGetTailThread;
        this->periodicGetTailThread = NULL;
    }
}

void PeriodicGetTailHandler::startPeriodicGetTail() {
    while (!this->stop_flag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(this->periodTimeMs));
        (*this->rootActor_)->anon_send(this->deltaCacheActor_, CacheDeltaAtom_v);
    }
}

}
