//
// Created by Yifei Yang on 11/20/21.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_PHYSICALPLAN_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_PHYSICALPLAN_H

#include <fpdb/executor/physical/PhysicalOp.h>
#include <utility>

using namespace std;

namespace fpdb::executor::physical {

class PhysicalPlan {
public:
  PhysicalPlan(const vector<shared_ptr<PhysicalOp>> &physicalOps);

  const vector<shared_ptr<PhysicalOp>> &getPhysicalOps() const;

private:
  vector<shared_ptr<PhysicalOp>> physicalOps_;
};

}


#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_PHYSICALPLAN_H
