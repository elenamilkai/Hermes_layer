//
// Created by matt on 5/12/19.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_COLLATE_COLLATEPOP_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_COLLATE_COLLATEPOP_H

#include <fpdb/executor/physical/PhysicalOp.h>
#include <fpdb/executor/physical/POpContext.h>
#include <fpdb/executor/message/TupleMessage.h>
#include <fpdb/executor/message/CompleteMessage.h>
#include <fpdb/tuple/TupleSet.h>
#include <arrow/api.h>
#include <string>
#include <memory>                  // for unique_ptr

namespace fpdb::tuple {
	class TupleSet;
}

namespace fpdb::executor::physical::collate {

class CollatePOp : public PhysicalOp {

public:
  explicit CollatePOp(std::string name,
                      std::vector<std::string> projectColumnNames,
                      int nodeId);
  CollatePOp() = default;
  CollatePOp(const CollatePOp&) = default;
  CollatePOp& operator=(const CollatePOp&) = default;
  ~CollatePOp() override = default;

  std::shared_ptr<TupleSet> tuples();

  void onReceive(const fpdb::executor::message::Envelope &message) override;
  void clear() override;
  std::string getTypeString() const override;

private:
  void onStart();
  void onComplete(const fpdb::executor::message::CompleteMessage &message);
  void onTuple(const fpdb::executor::message::TupleMessage& message);

  std::shared_ptr<TupleSet> tuples_;
  std::vector<std::shared_ptr<arrow::Table>> tables_;
  size_t tablesCutoff_ = 20;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, CollatePOp& op) {
    return f.object(op).fields(f.field("name", op.name_),
                               f.field("type", op.type_),
                               f.field("projectColumnNames", op.projectColumnNames_),
                               f.field("nodeId", op.nodeId_),
                               f.field("queryId", op.queryId_),
                               f.field("opContext", op.opContext_),
                               f.field("producers", op.producers_),
                               f.field("consumers", op.consumers_),
                               f.field("tablesCutoff", op.tablesCutoff_));
  }
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_COLLATE_COLLATEPOP_H
