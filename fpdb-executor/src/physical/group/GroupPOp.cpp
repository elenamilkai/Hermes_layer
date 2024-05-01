//
// Created by matt on 13/5/20.
//

#include <fpdb/executor/physical/group/GroupPOp.h>

using namespace fpdb::tuple;

namespace fpdb::executor::physical::group {

GroupPOp::GroupPOp(const string &name,
                   const vector<string> &projectColumnNames,
                   int nodeId,
                   const vector<string> &groupColumnNames,
                   const vector<shared_ptr<aggregate::AggregateFunction>> &aggregateFunctions) :
	PhysicalOp(name, GROUP, projectColumnNames, nodeId),
  kernel_(groupColumnNames, aggregateFunctions) {
}

std::string GroupPOp::getTypeString() const {
  return "GroupPOp";
}

void GroupPOp::onReceive(const Envelope &msg) {
  if (msg.message().type() == MessageType::START) {
	  this->onStart();
  } else if (msg.message().type() == MessageType::TUPLE) {
    auto tupleMessage = dynamic_cast<const TupleMessage &>(msg.message());
    this->onTuple(tupleMessage);
    //SPDLOG_CRITICAL("[{}] Received tuples of size {} from {}. ", name(), tupleMessage.tuples()->numRows(), msg.message().sender());
  } else if (msg.message().type() == MessageType::COMPLETE) {
    auto completeMessage = dynamic_cast<const CompleteMessage &>(msg.message());
    this->onComplete(completeMessage);
  } else {
    ctx()->notifyError("Unrecognized message type " + msg.message().getTypeString());
  }
}

void GroupPOp::onStart() {
//  SPDLOG_CRITICAL("Starting operator  |  name: '{}'", this->name());
}

void GroupPOp::onTuple(const TupleMessage &message) {
  /*const auto &tupleSet = message.tuples();
  auto expectedGroupResult = kernel_.group(*tupleSet);
  if(!expectedGroupResult)
    ctx()->notifyError(expectedGroupResult.error());*/
}

void GroupPOp::onComplete(const CompleteMessage &) {
  if (!ctx()->isComplete() && this->ctx()->operatorMap().allComplete(POpRelationshipType::Producer)) {

    /*auto expectedGroupedTupleSet = kernel_.finalise();
    if (!expectedGroupedTupleSet)
      ctx()->notifyError(expectedGroupedTupleSet.error());

    // Project using projectColumnNames
    auto expProjectTupleSet = expectedGroupedTupleSet.value()->projectExist(getProjectColumnNames());
    if (!expProjectTupleSet) {
      ctx()->notifyError(expProjectTupleSet.error());
    }

    shared_ptr<Message> tupleMessage = make_shared<TupleMessage>(expProjectTupleSet.value(), this->name());
    ctx()->tell(tupleMessage);*/

    ctx()->notifyComplete();
  }
}

void GroupPOp::clear() {
  kernel_.clear();
}

}
