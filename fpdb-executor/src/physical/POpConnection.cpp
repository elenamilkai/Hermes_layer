//
// Created by matt on 23/9/20.
//

#include <fpdb/executor/physical/POpConnection.h>
#include <utility>

namespace fpdb::executor::physical {

POpConnection::POpConnection(std::string name,
                             ::caf::actor actorHandle,
                             POpRelationshipType connectionType) :
	name_(std::move(name)),
	actorHandle_(std::move(actorHandle)),
	connectionType_(connectionType) {}

const std::string &POpConnection::getName() const {
  return name_;
}

const ::caf::actor &POpConnection::getActorHandle() const {
  return actorHandle_;
}

POpRelationshipType POpConnection::getConnectionType() const {
  return connectionType_;
}

}
