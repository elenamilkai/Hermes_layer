//
// Created by matt on 13/5/20.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_GROUP_GROUPPOP_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_GROUP_GROUPPOP_H

#include <fpdb/executor/physical/group/GroupKernel.h>
#include <fpdb/executor/physical/PhysicalOp.h>
#include <fpdb/executor/physical/aggregate/function/AggregateFunction.h>
#include <fpdb/executor/message/CompleteMessage.h>
#include <fpdb/executor/message/TupleMessage.h>
#include <string>
#include <vector>
#include <memory>

using namespace fpdb::executor::message;
using namespace std;

namespace fpdb::executor::physical::group {

/**
 * Group with aggregate operator
 *
 * This operator takes a list of column names to group by and a list of aggregate expressions to
 * compute for each group.
 *
 * A map is created from grouped values to computed aggregates. On the receipt of each tuple set
 * the aggregates are computed for each group and stored in the map.
 *
 * Upon completion of all upstream operators the final aggregates for each group are sent to downstream operators.
 */
class GroupPOp : public PhysicalOp {

public:
  GroupPOp(const string &name,
           const vector<string> &projectColumnNames,
           int nodeId,
           const vector<string> &groupColumnNames,
           const vector<shared_ptr<aggregate::AggregateFunction>> &aggregateFunctions);
  GroupPOp() = default;
  GroupPOp(const GroupPOp&) = default;
  GroupPOp& operator=(const GroupPOp&) = default;

  void onReceive(const Envelope &msg) override;
  void clear() override;
  std::string getTypeString() const override;

private:
  void onStart();
  void onTuple(const TupleMessage &msg);
  void onComplete(const CompleteMessage &msg);

  GroupKernel kernel_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, GroupPOp& op) {
    return f.object(op).fields(f.field("name", op.name_),
                               f.field("type", op.type_),
                               f.field("projectColumnNames", op.projectColumnNames_),
                               f.field("nodeId", op.nodeId_),
                               f.field("queryId", op.queryId_),
                               f.field("opContext", op.opContext_),
                               f.field("producers", op.producers_),
                               f.field("consumers", op.consumers_),
                               f.field("kernel", op.kernel_));
  }
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_GROUP_GROUPPOP_H
