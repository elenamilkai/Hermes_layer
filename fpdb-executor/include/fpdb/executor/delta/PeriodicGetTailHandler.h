//
// Created by xiaohans on 5/8/22.
//

#ifndef FPDB_PERIODICGETTAILHANDLER_H
#define FPDB_PERIODICGETTAILHANDLER_H

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <caf/all.hpp>

namespace fpdb::executor::delta {

class PeriodicGetTailHandler {
public:
    PeriodicGetTailHandler();

    ~PeriodicGetTailHandler();

    /**
     * Start a thread to periodically request DeltaCacheActor to get tail from logparser.
     * If another thread is still running, it will be stopped before the new thread is started.
     * (i.e. there would always be only one thread running periodic get tail)
     *
     * @param periodTimeMs is the period (in milliseconds) the request is sent
     */
    void start(int periodTimeMs, const ::caf::actor &deltaCacheActor,
               const std::shared_ptr<::caf::actor_system> &actorSystem);

    void stop();

private:
    /**
     *
     */
    void startPeriodicGetTail();

    std::unique_ptr<::caf::scoped_actor> rootActor_;
    ::caf::actor deltaCacheActor_;
    std::atomic<bool> stop_flag;
    int periodTimeMs;
    std::thread *periodicGetTailThread;
};

}


#endif //FPDB_PERIODICGETTAILHANDLER_H
