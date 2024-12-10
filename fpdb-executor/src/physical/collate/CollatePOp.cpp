//
// Created by matt on 5/12/19.
//

#include <fpdb/executor/physical/collate/CollatePOp.h>
#include <fpdb/executor/message/TupleMessage.h>
#include <fpdb/executor/message/CompleteMessage.h>
#include <arrow/table.h>               // for ConcatenateTables, Table (ptr ...
#include <arrow/pretty_print.h>
#include <vector>                      // for vector
#include <fpdb/delta/DeltaCacheMetadata.h>

namespace fpdb::executor::physical::collate {

CollatePOp::CollatePOp(std::string name,
                       std::vector<std::string> projectColumnNames,
                       int nodeId) :
  PhysicalOp(std::move(name), COLLATE, std::move(projectColumnNames), nodeId) {
}

std::string CollatePOp::getTypeString() const {
  return "CollatePOp";
}

void CollatePOp::onReceive(const fpdb::executor::message::Envelope &message) {
  if (message.message().type() == MessageType::START) {
    this->onStart();
  } else if (message.message().type() == MessageType::TUPLE) {
    auto tupleMessage = dynamic_cast<const fpdb::executor::message::TupleMessage &>(message.message());
    this->onTuple(tupleMessage);
  } else if (message.message().type() == MessageType::COMPLETE) {
    auto completeMessage = dynamic_cast<const fpdb::executor::message::CompleteMessage &>(message.message());
    this->onComplete(completeMessage);
  } else {
    ctx()->notifyError("Unrecognized message type " + message.message().getTypeString());
  }
}

void CollatePOp::onStart() {
///  SPDLOG_CRITICAL("Starting operator  |  name: '{}'", this->name());
}

void CollatePOp::onComplete(const fpdb::executor::message::CompleteMessage &) {
  if(!ctx()->isComplete() && ctx()->operatorMap().allComplete(POpRelationshipType::Producer)){
    /*if (!tables_.empty()) {
      tables_.push_back(tuples_->table());
      const arrow::Result<std::shared_ptr<arrow::Table>> &res = arrow::ConcatenateTables(tables_);
      if (!res.ok()) {
        ctx()->notifyError(res.status().message());
      }
      tuples_->table(*res);
      tables_.clear();
    }

    // make the order of output columns the same as the query specifies
    if (tuples_ && tuples_->valid()) {
      const auto &expTupleSet = tuples_->projectExist(getProjectColumnNames());
      if (!expTupleSet.has_value()) {
        ctx()->notifyError(expTupleSet.error());
      }
      tuples_ = expTupleSet.value();
    } else {
      tuples_ = TupleSet::makeWithEmptyTable();
    }*/
    ctx()->notifyComplete();
  }
}

void CollatePOp::onTuple(const fpdb::executor::message::TupleMessage &message) {
 /* if (!tuples_) {
    assert(message.tuples());
    tuples_ = message.tuples();
  } else {
    tables_.push_back(message.tuples()->table());
    if (tables_.size() > tablesCutoff_) {
      tables_.push_back(tuples_->table());
      const arrow::Result<std::shared_ptr<arrow::Table>> &res = arrow::ConcatenateTables(tables_);
      if (!res.ok()) {
        ctx()->notifyError(res.status().message());
      }
      tuples_->table(*res);
      tables_.clear();

    }
  }*/
}

std::shared_ptr<TupleSet> CollatePOp::tuples() {
  assert(tuples_);
  return tuples_;
}

void CollatePOp::clear() {
  tuples_.reset();
  tables_.clear();
}

}