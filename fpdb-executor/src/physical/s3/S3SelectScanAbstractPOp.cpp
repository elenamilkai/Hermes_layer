//
// Created by matt on 5/12/19.
//


#include <fpdb/executor/physical/s3/S3SelectScanAbstractPOp.h>
#include <fpdb/executor/physical/cache/CacheHelper.h>
#include <fpdb/executor/message/Message.h>
#include <fpdb/executor/message/TupleMessage.h>
#include <fpdb/executor/message/cache/LoadResponseMessage.h>
#include <fpdb/executor/message/delta/LoadDiskDeltasResponseMessage.h>
#include <fpdb/catalogue/s3/S3Partition.h>
#include <fpdb/tuple/TupleSet.h>
#include <arrow/type_fwd.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/core/utils/ratelimiter/DefaultRateLimiter.h>
#include <utility>
#include <cstdlib>
#include <arrow/csv/options.h>
#include <arrow/csv/reader.h>
#include <arrow/io/buffered.h>
#include <arrow/io/memory.h>
#include <arrow/type_fwd.h>
#include <parquet/arrow/reader.h>
#include <fpdb/executor/physical/Globals.h>
#include <fpdb/delta/DeltaCacheMetadata.h>
#include <arrow/io/api.h>
#include <aws/s3/model/HeadObjectRequest.h>

namespace Aws::Utils::RateLimits { class RateLimiterInterface; }
namespace arrow { class MemoryPool; }

using namespace Aws;
using namespace Aws::Auth;
using namespace Aws::Http;
using namespace Aws::Client;
using namespace Aws::S3;
using namespace Aws::S3::Model;

using namespace fpdb::executor::message;
using namespace fpdb::executor::physical::cache;
using namespace fpdb::cache;
using namespace fpdb::catalogue::s3;

namespace fpdb::executor::physical::s3 {

S3SelectScanAbstractPOp::S3SelectScanAbstractPOp(std::string name,
         POpType type,
         std::vector<std::string> projectColumnNames,
         int nodeId,
			   std::string s3Bucket,
			   std::string s3Object,
			   int64_t startOffset,
			   int64_t finishOffset,
         std::shared_ptr<Table> table,
         std::shared_ptr<fpdb::aws::AWSClient> awsClient,
			   bool scanOnStart,
			   bool toCache) :
	PhysicalOp(std::move(name), type, std::move(projectColumnNames), nodeId),
	s3Bucket_(std::move(s3Bucket)),
	s3Object_(std::move(s3Object)),
	startOffset_(startOffset),
	finishOffset_(finishOffset),
	table_(std::move(table)),
	awsClient_(std::move(awsClient)),
	columnsReadFromS3_(getProjectColumnNames().size()),
	scanOnStart_(scanOnStart),
	toCache_(toCache),
  splitReqLock_(std::make_shared<std::mutex>()) {
}

S3SelectScanAbstractPOp::S3SelectScanAbstractPOp() :
  awsClient_(fpdb::aws::AWSClient::daemonClient_),
  splitReqLock_(std::make_shared<std::mutex>()) {}

void S3SelectScanAbstractPOp::onReceive(const Envelope &message) {
  if (message.message().type() == MessageType::START) {
    this->onStart();
  } else if (message.message().type() == MessageType::SCAN) {
    auto scanMessage = dynamic_cast<const ScanMessage &>(message.message());
    this->onCacheLoadResponse(scanMessage);
  } else if (message.message().type() == MessageType::COMPLETE) {
    // Noop
  } else {
    ctx()->notifyError(fmt::format("Unrecognized message type: {}, {}", message.message().getTypeString(), name()));
  }
}

void S3SelectScanAbstractPOp::onStart() {
  if (scanOnStart_) {
    readAndSendTuples();
  }
}

void S3SelectScanAbstractPOp::readAndSendTuples() {
  if(!flag){
    /*int partition;
    auto partition_a = s3Object_.back();
    s3Object_.pop_back();
    auto partition_b = s3Object_.back();
    if (partition_b != '.') {
      s3Object_.pop_back();
      partition = (partition_b - '0') * 10 + partition_a - '0';
    }
    else{
      partition = partition_a - '0';
    }
    if(table_->getName()=="lineorder") {
      s3Object_ = "hattrick-sf50/parquet/lineorder_sharded/lineorder.parquet." + std::to_string(partition);
      SPDLOG_CRITICAL("{}, {}", s3Object_, partition);
    } else {
      s3Object_ = "hattrick-sf50/parquet/"+table_->getName()+".parquet";
      SPDLOG_CRITICAL("{}", s3Object_);
    }*/
    auto readTupleSet = readTuples();
    s3SelectScanStats_.outputBytes += readTupleSet->size();
    std::shared_ptr<Message> message = std::make_shared<TupleMessage>(readTupleSet, this->name());
    ctx()->tell(message);
    ctx()->notifyComplete();
  }
  else {
    std::vector<int> offsets = {0}; //{0, 50, 100, 150};
    std::string s3ObjectPath;
    int partition;
    if(table_->getName()=="lineorder") {
      auto partition_a = s3Object_.back();
      s3Object_.pop_back();
      auto partition_b = s3Object_.back();
      if (partition_b != '.') {
        s3Object_.pop_back();
        partition = (partition_b - '0') * 10 + partition_a - '0';
      } else {
        partition = partition_a - '0';
      }
      s3ObjectPath = s3Object_;
    }
    else {
      s3ObjectPath = s3Object_;
    }
    if(table_->getName()=="lineorder"){
      for (auto offset : offsets) {
        auto partNum = partition + offset;
        s3Object_ = s3ObjectPath + std::to_string(partNum);
        GetObjectRequest getObjectRequest;
        getObjectRequest.SetBucket(Aws::String(s3Bucket_));
        getObjectRequest.SetKey(Aws::String(s3Object_));
        std::chrono::steady_clock::time_point startTransferTime = std::chrono::steady_clock::now();
        GetObjectOutcome getObjectOutcome;
        int maxRetryAttempts = 100;
        int attempts = 0;
        while (true) {
          attempts++;
          getObjectOutcome = awsClient_->getS3Client()->GetObject(getObjectRequest);
          if (getObjectOutcome.IsSuccess()) {
            break;
          }
          if (attempts > maxRetryAttempts) {
            const auto &err = getObjectOutcome.GetError();
            ctx()->notifyError(fmt::format("{}, {} after {} retries", err.GetMessage(), name(), maxRetryAttempts));
          }
          // Something went wrong with AWS API on our end or remotely, wait and try again
          std::this_thread::sleep_for(std::chrono::milliseconds(minimumSleepRetryTimeMS));
        }
        std::chrono::steady_clock::time_point stopTransferTime = std::chrono::steady_clock::now();
        auto transferTime = std::chrono::duration_cast<std::chrono::nanoseconds>(stopTransferTime - startTransferTime).count();
        splitReqLock_->lock();
        s3SelectScanStats_.getTransferTimeNS += transferTime;
        s3SelectScanStats_.numRequests++;
        splitReqLock_->unlock();

        std::shared_ptr<TupleSet> tupleSet;
        auto getResult = getObjectOutcome.GetResultWithOwnership();
        int64_t resultSize = getResult.GetContentLength();
        splitReqLock_->lock();
        s3SelectScanStats_.processedBytes += resultSize;
        s3SelectScanStats_.returnedBytes += resultSize;
        splitReqLock_->unlock();
        std::vector<std::shared_ptr<arrow::Field>> fields;
        for (const auto &column : getProjectColumnNames()) {
          fields.emplace_back(::arrow::field(column, table_->getSchema()->GetFieldByName(column)->type()));
        }
        auto outputSchema = std::make_shared<::arrow::Schema>(fields);
        while (true) {
          if (GetConvertLock.try_lock()) {
            if (activeGetConversions < maxConcurrentArrowConversions) {
              activeGetConversions++;
              GetConvertLock.unlock();
              break;
            } else {
              GetConvertLock.unlock();
            }
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(rand() % variableSleepRetryTimeMS + minimumSleepRetryTimeMS));
        }
        std::chrono::steady_clock::time_point startConversionTime = std::chrono::steady_clock::now();
        Aws::IOStream &retrievedFile = getResult.GetBody();

        std::string parquetFileString(std::istreambuf_iterator<char>(retrievedFile), {});
        auto bufferedReader = std::make_shared<arrow::io::BufferReader>(parquetFileString);

        std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
        arrow::MemoryPool *pool = arrow::default_memory_pool();
        arrow::Status st = parquet::arrow::OpenFile(bufferedReader, pool, &arrow_reader);

        arrow_reader->set_use_threads(false);
        // Get the Parquet schema
        std::shared_ptr<arrow::Schema> schema;
        arrow_reader->GetSchema(&schema);

        // only read the needed columns from the file
        std::vector<int> neededColumnIndices;
        for (const auto &fieldName : getProjectColumnNames()) {
          neededColumnIndices.emplace_back(table_->getSchema()->GetFieldIndex(fieldName));
        }

        auto row_groups = arrow_reader->num_row_groups();
        vector<int> rowGroupsIndices;
        for (int i = 0; i < row_groups; i++) {
          rowGroupsIndices.emplace_back(i);
        }
        arrow_reader->set_batch_size(DefaultChunkSize);
        // Read the Parquet file in batches
        std::shared_ptr<arrow::RecordBatchReader> batch_reader;
        arrow_reader->GetRecordBatchReader(rowGroupsIndices, neededColumnIndices, &batch_reader);
        std::shared_ptr<arrow::RecordBatch> batch;
        std::shared_ptr<arrow::Table> table;
        int total_size = 0;
        while (true) {
          auto status = batch_reader->ReadNext(&batch);
          if (!status.ok()) {
            ctx()->notifyError(fmt::format("Error reading parquet batches {}\nError: {}", name(), status.message()));
          }
          if (batch == nullptr) {
            break;
          }
          table = *arrow::Table::FromRecordBatches({batch});
          auto readTupleSet = TupleSet::make(table);
          std::shared_ptr<Message> message = std::make_shared<TupleMessage>(readTupleSet, this->name());
          ctx()->tell(message);
          total_size += readTupleSet->size();
        }
        s3SelectScanStats_.outputBytes += total_size;
        GetConvertLock.lock();
        activeGetConversions--;
        GetConvertLock.unlock();
        std::chrono::steady_clock::time_point stopConversionTime = std::chrono::steady_clock::now();
        auto conversionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(stopConversionTime - startConversionTime).count();
        splitReqLock_->lock();
        s3SelectScanStats_.getConvertTimeNS += conversionTime;
        splitReqLock_->unlock();
      }
    } else {
      s3Object_ = s3ObjectPath;
      GetObjectRequest getObjectRequest;
      getObjectRequest.SetBucket(Aws::String(s3Bucket_));
      getObjectRequest.SetKey(Aws::String(s3Object_));
      std::chrono::steady_clock::time_point startTransferTime = std::chrono::steady_clock::now();
      GetObjectOutcome getObjectOutcome;
      int maxRetryAttempts = 100;
      int attempts = 0;
      while (true) {
        attempts++;
        getObjectOutcome = awsClient_->getS3Client()->GetObject(getObjectRequest);
        if (getObjectOutcome.IsSuccess()) {
          break;
        }
        if (attempts > maxRetryAttempts) {
          const auto &err = getObjectOutcome.GetError();
          ctx()->notifyError(fmt::format("{}, {} after {} retries", err.GetMessage(), name(), maxRetryAttempts));
        }
        // Something went wrong with AWS API on our end or remotely, wait and try again
        std::this_thread::sleep_for(std::chrono::milliseconds(minimumSleepRetryTimeMS));
      }
      std::chrono::steady_clock::time_point stopTransferTime = std::chrono::steady_clock::now();
      auto transferTime = std::chrono::duration_cast<std::chrono::nanoseconds>(stopTransferTime - startTransferTime).count();
      splitReqLock_->lock();
      s3SelectScanStats_.getTransferTimeNS += transferTime;
      s3SelectScanStats_.numRequests++;
      splitReqLock_->unlock();

      std::shared_ptr<TupleSet> tupleSet;
      auto getResult = getObjectOutcome.GetResultWithOwnership();
      int64_t resultSize = getResult.GetContentLength();
      splitReqLock_->lock();
      s3SelectScanStats_.processedBytes += resultSize;
      s3SelectScanStats_.returnedBytes += resultSize;
      splitReqLock_->unlock();
      std::vector<std::shared_ptr<arrow::Field>> fields;
      for (const auto &column : getProjectColumnNames()) {
        fields.emplace_back(::arrow::field(column, table_->getSchema()->GetFieldByName(column)->type()));
      }
      auto outputSchema = std::make_shared<::arrow::Schema>(fields);
      while (true) {
        if (GetConvertLock.try_lock()) {
          if (activeGetConversions < maxConcurrentArrowConversions) {
            activeGetConversions++;
            GetConvertLock.unlock();
            break;
          } else {
            GetConvertLock.unlock();
          }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(rand() % variableSleepRetryTimeMS + minimumSleepRetryTimeMS));
      }
      std::chrono::steady_clock::time_point startConversionTime = std::chrono::steady_clock::now();
      Aws::IOStream &retrievedFile = getResult.GetBody();

      std::string parquetFileString(std::istreambuf_iterator<char>(retrievedFile), {});
      auto bufferedReader = std::make_shared<arrow::io::BufferReader>(parquetFileString);

      std::unique_ptr<parquet::arrow::FileReader> arrow_reader;
      arrow::MemoryPool *pool = arrow::default_memory_pool();
      arrow::Status st = parquet::arrow::OpenFile(bufferedReader, pool, &arrow_reader);

      arrow_reader->set_use_threads(false);
      // Get the Parquet schema
      std::shared_ptr<arrow::Schema> schema;
      arrow_reader->GetSchema(&schema);

      // only read the needed columns from the file
      std::vector<int> neededColumnIndices;
      for (const auto &fieldName : getProjectColumnNames()) {
        neededColumnIndices.emplace_back(table_->getSchema()->GetFieldIndex(fieldName));
      }

      auto row_groups = arrow_reader->num_row_groups();
      vector<int> rowGroupsIndices;
      for (int i = 0; i < row_groups; i++) {
        rowGroupsIndices.emplace_back(i);
      }
      arrow_reader->set_batch_size(DefaultChunkSize);
      // Read the Parquet file in batches
      std::shared_ptr<arrow::RecordBatchReader> batch_reader;
      arrow_reader->GetRecordBatchReader(rowGroupsIndices, neededColumnIndices, &batch_reader);
      std::shared_ptr<arrow::RecordBatch> batch;
      std::shared_ptr<arrow::Table> table;
      int total_size = 0;
      while (true) {
        auto status = batch_reader->ReadNext(&batch);
        if (!status.ok()) {
          ctx()->notifyError(fmt::format("Error reading parquet batches {}\nError: {}", name(), status.message()));
        }
        if (batch == nullptr) {
          break;
        }
        table = *arrow::Table::FromRecordBatches({batch});
        auto readTupleSet = TupleSet::make(table);
        std::shared_ptr<Message> message = std::make_shared<TupleMessage>(readTupleSet, this->name());
        ctx()->tell(message);
        total_size += readTupleSet->size();
      }
      s3SelectScanStats_.outputBytes += total_size;
      GetConvertLock.lock();
      activeGetConversions--;
      GetConvertLock.unlock();
      std::chrono::steady_clock::time_point stopConversionTime = std::chrono::steady_clock::now();
      auto conversionTime = std::chrono::duration_cast<std::chrono::nanoseconds>(stopConversionTime - startConversionTime).count();
      splitReqLock_->lock();
      s3SelectScanStats_.getConvertTimeNS += conversionTime;
      splitReqLock_->unlock();
    }

    ctx()->notifyComplete();
  }
}

void S3SelectScanAbstractPOp::put(const std::shared_ptr<TupleSet> &tupleSet) {
  auto columnNames = getProjectColumnNames();

  for (int columnIndex = 0; columnIndex < tupleSet->numColumns(); ++columnIndex) {

    auto columnName = columnNames.at(columnIndex);
    auto readColumn = tupleSet->getColumnByIndex(columnIndex).value();
    auto canonicalColumnName = ColumnName::canonicalize(columnName);
    readColumn->setName(canonicalColumnName);

    auto bufferedColumnArrays = columnsReadFromS3_[columnIndex];

    if (bufferedColumnArrays == nullptr) {
      bufferedColumnArrays = std::make_shared<std::pair<std::string, ::arrow::ArrayVector>>(readColumn->getName(),
                                                                                            readColumn->getArrowArray()->chunks());
      columnsReadFromS3_[columnIndex] = bufferedColumnArrays;
    } else {
      // Add the read chunks to this buffered columns chunk vector
      for (int chunkIndex = 0; chunkIndex < readColumn->getArrowArray()->num_chunks(); ++chunkIndex) {
        auto readChunk = readColumn->getArrowArray()->chunk(chunkIndex);
        bufferedColumnArrays->second.emplace_back(readChunk);
      }
    }
  }
}

void S3SelectScanAbstractPOp::onCacheLoadResponse(const ScanMessage &message) {
  processScanMessage(message);

  if (message.isResultNeeded()) {
    readAndSendTuples();
  }

  else {
    auto emptyTupleSet = TupleSet::makeWithEmptyTable();
    std::shared_ptr<Message>
            responseMessage = std::make_shared<TupleMessage>(emptyTupleSet, this->name());
    ctx()->tell(responseMessage);
    SPDLOG_DEBUG(fmt::format("Finished because result not needed: {}/{}", s3Bucket_, s3Object_));

    /**
     * Here caching is asynchronous,
     * so need to backup ctx first, because it's a weak_ptr, after query finishing will be destroyed
     * even no use of this "ctxBackup" is ok
     */
    auto ctxBackup = ctx();

    ctx()->notifyComplete();

    // just to cache
    readTuples();
  }
}

void S3SelectScanAbstractPOp::requestStoreSegmentsInCache(const std::shared_ptr<TupleSet> &tupleSet) {
  auto partition = std::make_shared<S3Partition>(s3Bucket_, s3Object_, finishOffset_ - startOffset_);
  CacheHelper::requestStoreSegmentsInCache(tupleSet, partition, startOffset_, finishOffset_, name(), ctx());
}

S3SelectScanStats S3SelectScanAbstractPOp::getS3SelectScanStats() {
  return s3SelectScanStats_;
}

}
