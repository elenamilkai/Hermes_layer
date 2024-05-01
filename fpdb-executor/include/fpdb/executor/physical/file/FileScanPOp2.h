//
// Created by matt on 23/9/20.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANPOP2_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANPOP2_H

#include <fpdb/executor/physical/Forward.h>
#include <fpdb/executor/physical/file/FileScanPOp.h>
#include <fpdb/executor/physical/POpActor2.h>
#include <fpdb/executor/cache/SegmentCacheActor.h>
#include <fpdb/executor/message/TupleMessage.h>
#include <fpdb/executor/message/cache/StoreRequestMessage.h>
#include <fpdb/executor/serialization/MessageSerializer.h>
#include <fpdb/catalogue/local-fs/LocalFSPartition.h>
#include <fpdb/caf/CAFUtil.h>
#include <utility>

using namespace fpdb::executor::message;
using namespace fpdb::executor::cache;
using namespace fpdb::tuple;

CAF_BEGIN_TYPE_ID_BLOCK(FileScanPOp2, fpdb::caf::CAFUtil::FileScanPOp2_first_custom_type_id)
CAF_ADD_ATOM(FileScanPOp2, ScanAtom)
CAF_ADD_TYPE_ID(FileScanPOp2, (std::vector<std::string>))
CAF_END_TYPE_ID_BLOCK(FileScanPOp2)

namespace fpdb::executor::physical::file {

using FileScanActor = POpActor2::extend_with<::caf::typed_actor<
	::caf::reacts_to<ScanAtom, std::vector<std::string>, bool>>>;

using FileScanStatefulActor = FileScanActor::stateful_pointer<FileScanState>;

class FileScanState : public POpActorState<FileScanStatefulActor> {
public:
  void setState(FileScanStatefulActor actor,
				std::string name,
				const std::string &filePath,
				FileType fileType,
				const std::vector<std::string> &columnNames,
				unsigned long startOffset,
				unsigned long finishOffset,
				long queryId,
				const ::caf::actor &rootActorHandle,
				const ::caf::actor &segmentCacheActorHandle,
				bool scanOnStart = false) {

	POpActorState::setBaseState(actor, std::move(name), queryId, rootActorHandle, segmentCacheActorHandle);

	auto canonicalColumnNames = ColumnName::canonicalize(columnNames);

	filePath_ = filePath;
	fileType_ = fileType;
	columnNames_ = canonicalColumnNames;
	startOffset_ = startOffset;
	finishOffset_ = finishOffset;
	scanOnStart_ = scanOnStart;

	kernel_ = FileScanKernel::make(filePath, fileType, startOffset, finishOffset);
  }

  template<class... Handlers>
  FileScanActor::behavior_type makeBehavior(FileScanStatefulActor actor, Handlers... handlers) {
	return POpActorState::makeBaseBehavior(
		actor,
		[=](ScanAtom, const std::vector<std::string> &columnNames, bool /*resultNeeded*/) {
		  process(actor,
				  [=](const ::caf::strong_actor_ptr &messageSender) {
					return onScan(actor,
								  messageSender,
								  columnNames);
				  });
		},
		std::move(handlers)...
	);
  }

private:
  std::string filePath_;
  FileType fileType_;
  std::vector<std::string> columnNames_;
  unsigned long startOffset_;
  unsigned long finishOffset_;
  bool scanOnStart_ = false;

  FileScanKernel kernel_;

protected:

  tl::expected<void, std::string>
  onStart(FileScanStatefulActor actor, const ::caf::strong_actor_ptr & /*messageSender*/) override {
	if (scanOnStart_) {
	  return readAndSendTuples(actor, columnNames_)
		  .and_then([=]() { return notifyComplete(actor); });
	}
	return {};
  }

  tl::expected<void, std::string>
  onComplete(FileScanStatefulActor actor, const ::caf::strong_actor_ptr & /*messageSender*/) override {
	if (!isComplete() && isAllProducersComplete()) {
	  return notifyComplete(actor);
	}
	return {};
  }

  tl::expected<void, std::string>
  onEnvelope(FileScanStatefulActor actor,
			 const ::caf::strong_actor_ptr &messageSender,
			 const Envelope &envelope) override {
	if (envelope.message().type() == MessageType::SCAN) {
	  auto scanMessage = dynamic_cast<const ScanMessage &>(envelope.message());
	  return this->onScan(actor, messageSender, scanMessage.getColumnNames());
	} else {
	  return tl::make_unexpected(fmt::format("Unrecognized message type {}", envelope.message().getTypeString()));
	}
  }

private:

  [[nodiscard]] tl::expected<void, std::string>
  onScan(FileScanStatefulActor actor,
		 const ::caf::strong_actor_ptr &messageSender,
		 const std::vector<std::string> &columnsToScan) {

	SPDLOG_DEBUG("[Actor {} ('{}')]  Scan  |  sender: {}", actor->id(),
				 actor->name(), to_string(messageSender));

	return readAndSendTuples(actor, columnsToScan);
  }

  void requestStoreSegmentsInCache(FileScanStatefulActor actor, const std::shared_ptr<TupleSet> &tupleSet) {

	assert(tupleSet);
	assert(startOffset_ >= 0);
	assert(finishOffset_ > startOffset_);

	auto partition = std::make_shared<catalogue::local_fs::LocalFSPartition>(kernel_.getPath());

	std::unordered_map<std::shared_ptr<SegmentKey>, std::shared_ptr<SegmentData>> segmentsToStore;
	for (int64_t c = 0; c < tupleSet->numColumns(); ++c) {
	  auto column = tupleSet->getColumnByIndex(c).value();
	  auto segmentKey = SegmentKey::make(partition,
										 column->getName(),
										 SegmentRange::make(startOffset_, finishOffset_),
										 SegmentMetadata::make(column->size()));
	  auto segmentData = SegmentData::make(column);

	  segmentsToStore.emplace(segmentKey, segmentData);
	}

	anonymousSend(actor,
				  getSegmentCacheActorHandle().value(),
				  StoreAtom_v,
				  StoreRequestMessage::make(segmentsToStore, name));
  }

  [[nodiscard]] tl::expected<void, std::string>
  readAndSendTuples(FileScanStatefulActor actor, const std::vector<std::string> &columnNames) {
	// Read the columns not present in the cache
	/*
	 * FIXME: Should support reading the file in pieces
	 */

	std::shared_ptr<TupleSet> readTupleSet;
	if (columnNames.empty()) {
	  readTupleSet = TupleSet::makeWithEmptyTable();
	} else {
	  auto expectedReadTupleSet = kernel_.scan(columnNames);
	  readTupleSet = expectedReadTupleSet.value();

	  // Store the read columns in the cache
	  requestStoreSegmentsInCache(actor, readTupleSet);
	}

	std::shared_ptr<Message> message = std::make_shared<TupleMessage>(readTupleSet, this->name);
	return tell(actor, Envelope(message));
  }

};

FileScanActor::behavior_type FileScanFunctor(FileScanStatefulActor actor,
											 std::string name,
											 const std::string &filePath,
											 FileType fileType,
											 const std::vector<std::string> &columnNames,
											 unsigned long startOffset,
											 unsigned long finishOffset,
											 long queryId,
											 const ::caf::actor &rootActorHandle,
											 const ::caf::actor &segmentCacheActorHandle,
											 bool scanOnStart = false);

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANPOP2_H
