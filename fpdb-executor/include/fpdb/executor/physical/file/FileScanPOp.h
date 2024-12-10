//
// Created by matt on 12/12/19.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANPOP_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANPOP_H

#include <fpdb/executor/physical/file/FileScanKernel.h>
#include <fpdb/executor/physical/PhysicalOp.h>
#include <fpdb/executor/message/CompleteMessage.h>
#include <fpdb/executor/message/ScanMessage.h>
#include <fpdb/executor/message/cache/LoadResponseMessage.h>
#include <string>

using namespace fpdb::executor::message;
using namespace fpdb::tuple;

namespace fpdb::executor::physical::file {

class FileScanPOp : public PhysicalOp {

public:
  FileScanPOp(std::string name,
       std::vector<std::string> columnNames,
       int nodeId,
		   const std::string &filePath,
		   FileType fileType,
		   unsigned long startOffset,
		   unsigned long finishOffset,
		   bool scanOnStart = false);
  FileScanPOp() = default;
  FileScanPOp(const FileScanPOp&) = default;
  FileScanPOp& operator=(const FileScanPOp&) = default;

  [[nodiscard]] const FileScanKernel &getKernel() const;
  [[nodiscard]] bool isScanOnStart() const;

  void onReceive(const Envelope &message) override;
  void clear() override;
  std::string getTypeString() const override;

private:
  void onStart();
  void onCacheLoadResponse(const ScanMessage &Message);
  void onComplete(const CompleteMessage &message);
  void requestStoreSegmentsInCache(const std::shared_ptr<TupleSet> &tupleSet);
  void readAndSendTuples(const std::vector<std::string> &columnNames);

  bool scanOnStart_;
  FileScanKernel kernel_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, FileScanPOp& op) {
    return f.object(op).fields(f.field("name", op.name_),
                               f.field("type", op.type_),
                               f.field("projectColumnNames", op.projectColumnNames_),
                               f.field("nodeId", op.nodeId_),
                               f.field("queryId", op.queryId_),
                               f.field("opContext", op.opContext_),
                               f.field("producers", op.producers_),
                               f.field("consumers", op.consumers_),
                               f.field("scanOnStart", op.scanOnStart_),
                               f.field("kernel", op.kernel_));
  }
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANPOP_H
