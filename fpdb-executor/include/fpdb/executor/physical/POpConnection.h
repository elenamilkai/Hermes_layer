//
// Created by matt on 23/9/20.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_POPCONNECTION_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_POPCONNECTION_H

#include <fpdb/executor/physical/POpRelationshipType.h>
#include <caf/all.hpp>
#include <string>

namespace fpdb::executor::physical {

class POpConnection {
public:
  POpConnection(std::string Name, ::caf::actor ActorHandle, POpRelationshipType ConnectionType);
  POpConnection() = default;
  POpConnection(const POpConnection&) = default;
  POpConnection& operator=(const POpConnection&) = default;

  const std::string &getName() const;
  const ::caf::actor &getActorHandle() const;
  POpRelationshipType getConnectionType() const;

private:
  std::string name_;
  ::caf::actor actorHandle_;
  POpRelationshipType connectionType_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, POpConnection& con) {
    return f.object(con).fields(f.field("name", con.name_),
                                f.field("actorHandle", con.actorHandle_),
                                f.field("connectionType", con.connectionType_));
  }
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_POPCONNECTION_H
