//
// Created by matt on 11/12/19.
//

#include <fpdb/executor/message/TupleMessage.h>
#include <utility>

namespace fpdb::executor::message {

TupleMessage::TupleMessage(std::shared_ptr<TupleSet> tuples,
                           std::string sender) :
  Message(TUPLE, std::move(sender)),
  tuples_(std::move(tuples)) {
}

std::string TupleMessage::getTypeString() const {
  return "TupleMessage";
}

const std::shared_ptr<TupleSet>& TupleMessage::tuples() const {
  return tuples_;
}

}