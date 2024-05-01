//
// Created by Yifei Yang on 11/20/21.
//

#include <fpdb/executor/physical/PhysicalPlan.h>

namespace fpdb::executor::physical {

PhysicalPlan::PhysicalPlan(const vector<shared_ptr<PhysicalOp>> &physicalOps)
        : physicalOps_(physicalOps) {}

const vector<shared_ptr<PhysicalOp>> &PhysicalPlan::getPhysicalOps() const {
  return physicalOps_;
}

}
