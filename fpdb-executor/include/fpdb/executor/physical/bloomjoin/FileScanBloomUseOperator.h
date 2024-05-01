//
// Created by matt on 5/8/20.
//

#ifndef FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_FILESCANBLOOMUSEOPERATOR_H
#define FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_FILESCANBLOOMUSEOPERATOR_H

#include <fpdb/core/Operator.h>
#include <fpdb/core/message/CompleteMessage.h>
#include <fpdb/core/message/TupleMessage.h>
#include <fpdb/pushdown/scan/ScanMessage.h>

#include "FileScanBloomUseKernel.h"
#include "BloomFilterMessage.h"

using namespace fpdb::core;
using namespace fpdb::core::message;
using namespace fpdb::pushdown::scan;

namespace fpdb::pushdown::bloomjoin {

class FileScanBloomUseOperator : public Operator {

public:
  FileScanBloomUseOperator(const std::string &name,
                           const std::string &filePath,
                           const std::vector<std::string> &columnNames,
                           int startOffset,
                           int finishOffset,
                           const std::string &columnName,
                           long queryId) :
	  Operator(name, "FileScanBloomUseOperator", queryId),
	  kernel_(FileScanBloomUseKernel::make(filePath, columnNames, startOffset, finishOffset, columnName)) {
  }

  static std::shared_ptr<FileScanBloomUseOperator> make(const std::string &name,
                                                        const std::string &filePath,
                                                        const std::vector<std::string> &columnNames,
                                                        int startOffset,
                                                        int finishOffset,
                                                        const std::string &columnName,
                                                        long queryId = 0) {

	return std::make_shared<FileScanBloomUseOperator>(name,
													  filePath,
													  columnNames,
													  startOffset,
													  finishOffset,
													  columnName,
													  queryId);
  }

  void onReceive(const Envelope &msg) override {
	if (msg.message().type() == "StartMessage") {
	  this->onStart();
	} else if (msg.message().type() == "ScanMessage") {
	  auto scanMessage = dynamic_cast<const ScanMessage &>(msg.message());
	  this->onCacheLoadResponse(scanMessage);
	} else if (msg.message().type() == "BloomFilterMessage") {
	  auto bloomFilterMessage = dynamic_cast<const BloomFilterMessage &>(msg.message());
	  this->onBloomFilter(bloomFilterMessage);
	} else if (msg.message().type() == "CompleteMessage") {
	  auto completeMessage = dynamic_cast<const CompleteMessage &>(msg.message());
	  this->onComplete(completeMessage);
	} else {
	  // FIXME: Propagate error properly
	  throw std::runtime_error("Unrecognized message type " + msg.message().type());
	}
  }

private:

  std::shared_ptr<FileScanBloomUseKernel> kernel_;

  void onStart() {
	SPDLOG_DEBUG("Starting operator  |  name: '{}'", this->name());
  }

  void onCacheLoadResponse(const ScanMessage &msg) {
	auto result = kernel_->scan(msg.getColumnNames());
	if (!result)
	  throw std::runtime_error(result.error());
  }

  void onBloomFilter(const BloomFilterMessage &msg) {
	auto result = kernel_->setBloomFilter(msg.getBloomFilter());
	if (!result)
	  throw std::runtime_error(result.error());
  }

  void onComplete(const CompleteMessage & /*msg*/) {
	if (weakCtx()->operatorMap().allComplete(OperatorRelationshipType::Producer)) {

	  auto filterResult = kernel_->filter();
	  if (!filterResult)
		throw std::runtime_error(filterResult.error());

	  auto expectedTupleSet = kernel_->getTupleSet();
	  if (!expectedTupleSet)
		throw std::runtime_error("No Tuple Set");
	  auto tupleSet = expectedTupleSet.value();

	  std::shared_ptr<Message>
		  tupleSetMessage = std::make_shared<TupleMessage>(tupleSet->toTupleSetV1(), name());
	  weakCtx()->tell(tupleSetMessage);

	  weakCtx()->notifyComplete();
	}
  }

};

}

#endif //FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_FILESCANBLOOMUSEOPERATOR_H
