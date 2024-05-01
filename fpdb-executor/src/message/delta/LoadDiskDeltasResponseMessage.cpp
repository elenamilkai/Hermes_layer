//
// Created by Elena Milkai on 3/8/22.
//

#include <fpdb/executor/message/delta/LoadDiskDeltasResponseMessage.h>

using namespace fpdb::executor::message;
using namespace fpdb::tuple;

LoadDiskDeltasResponseMessage::LoadDiskDeltasResponseMessage(std::shared_ptr<TupleSet> diskDeltasTuples,
                                                             int timestamp,
                                                             std::string sender) :
        Message(DISK_DELTAS, sender),
        diskDeltasTuples_(std::move(diskDeltasTuples)),
        timestamp_(std::move(timestamp)) {}

std::shared_ptr<LoadDiskDeltasResponseMessage> LoadDiskDeltasResponseMessage::make(std::shared_ptr<TupleSet> diskDeltasTuples,
                                                                         int timestamp,
                                                                         std::string sender) {
    return std::make_shared<LoadDiskDeltasResponseMessage>(std::move(diskDeltasTuples),
                                                           std::move(timestamp),
                                                           sender);
}

std::string LoadDiskDeltasResponseMessage::getTypeString() const {
    return "LoadDiskDeltasResponseMessage";
}

std::shared_ptr<TupleSet> LoadDiskDeltasResponseMessage::getTuples() const {
    return diskDeltasTuples_;
}

int LoadDiskDeltasResponseMessage::getTimestamp() const {
    return timestamp_;
}

std::string LoadDiskDeltasResponseMessage::toString() const {
    std::string s = "Disk Deltas : {";
    s += fmt::format("Num of tuples: {}, timestamp: {}",
                         std::to_string(diskDeltasTuples_->size()), std::to_string(timestamp_));
    s += "}";
    return s;
}
