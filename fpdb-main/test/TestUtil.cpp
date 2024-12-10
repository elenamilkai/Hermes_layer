//
// Created by Yifei Yang on 11/30/21.
//

#include "TestUtil.h"
#include <fpdb/main/CAFInit.h>
#include <fpdb/main/ExecConfig.h>
#include <fpdb/executor/physical/transform/PrePToPTransformer.h>
#include <fpdb/calcite/CalciteConfig.h>
#include <fpdb/plan/calcite/CalcitePlanJsonDeserializer.h>
#include <fpdb/catalogue/s3/S3CatalogueEntryReader.h>
#include <fpdb/aws/AWSConfig.h>
#include <fpdb/util/Util.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <fpdb/delta/DeltaCacheMetadata.h>
#include <fpdb/hermesserver/Server.h>
#include <fpdb/tuple/serialization/ArrowSerializer.h>
#include <parquet/arrow/writer.h>
#include <queue>

#include <iostream>
#include <cstring>
#include <sys/ipc.h>
#include <sys/shm.h>

using namespace fpdb::executor::physical;
using namespace fpdb::plan::calcite;
using namespace fpdb::catalogue::s3;
using namespace fpdb::util;
using namespace fpdb::hermesserver;
using namespace fpdb::tuple;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;
shared_ptr<Server> hermes;
std::queue<std::shared_ptr<arrow::Table>> lo_batchQueue;
std::queue<std::shared_ptr<arrow::Table>> c_batchQueue;
std::queue<std::shared_ptr<arrow::Table>> s_batchQueue;
std::queue<std::shared_ptr<arrow::Table>> p_batchQueue;
std::queue<std::shared_ptr<arrow::Table>> d_batchQueue;
std::mutex lo_mtx_;
std::mutex c_mtx_;
std::mutex p_mtx_;
std::mutex s_mtx_;
std::mutex d_mtx_;
std::condition_variable lo_vBatchReady;
std::condition_variable c_vBatchReady;
std::condition_variable s_vBatchReady;
std::condition_variable p_vBatchReady;
std::condition_variable d_vBatchReady;

namespace fpdb::main::test {

TestUtil::TestUtil(const string &schemaName,
                   const vector<string> &queryFileNames,
                   int parallelDegree,
                   bool isDistributed) : schemaName_(schemaName),
                                         queryFileNames_(queryFileNames),
                                         parallelDegree_(parallelDegree),
                                         isDistributed_(isDistributed) {}

bool TestUtil::e2eNoStartCalciteServer(const string &schemaName,
                                       const vector<string> &queryFileNames,
                                       int parallelDegree,
                                       bool isDistributed) {
  //TestUtil testUtil(schemaName, queryFileNames, parallelDegree, isDistributed);
  shared_ptr<TestUtil> testUtil = make_shared<TestUtil>(schemaName, queryFileNames, parallelDegree, isDistributed);
  hermes = make_shared<Server>(testUtil);
  try {
    testUtil->runTest();
    return true;
  } catch (const runtime_error &err) {
    cout << err.what() << endl;
    return false;
  }
}

void TestUtil::runTest() {
  spdlog::set_level(spdlog::level::info);
  // AWS client
  makeAWSClient();
  // Catalogue entry
  makeCatalogueEntry();
  // Calcite client
  makeCalciteClient();


  // mode and caching policy
  mode_ = Mode::pullupMode();
  cachingPolicy_ = nullptr;

  // create the executor
  makeExecutor();
  /*int qcount = 0;
  auto startTime = chrono::steady_clock::now();
  int testDuration = 2600;
  for (const auto &queryFileName: queryFileNames_) {
    sleep(0);
    if(runTime(startTime, testDuration)) {
      if (qcount == 1) {
        flag = true;   // first query generates delete map with the old version
      }
      executeQueryFile(queryFileName, qcount);
      qcount++;
    }
    else{
      break;
    }
    std::queue<shared_ptr<arrow::Table>> lo_empty;
    std::swap( lo_batchQueue, lo_empty );
    std::queue<shared_ptr<arrow::Table>> c_empty;
    std::swap( c_batchQueue, c_empty );
    std::queue<shared_ptr<arrow::Table>> s_empty;
    std::swap( s_batchQueue, s_empty );
    std::queue<shared_ptr<arrow::Table>> p_empty;
    std::swap( p_batchQueue, p_empty );
    std::queue<shared_ptr<arrow::Table>> d_empty;
    std::swap( d_batchQueue, d_empty );
  }
  auto at = (double) qcount/testDuration;
  cout << "Total number of queries: " << qcount  << endl;
  cout << "Analytical throughput: " << at << " qps" <<  endl;*/

  // start hermes server
  hermes->start();
  //stop();
}

void TestUtil::makeAWSClient() {
  awsClient_ = make_shared<AWSClient>(
      make_shared<AWSConfig>(fpdb::aws::S3, 0));
  awsClient_->init();
}

void TestUtil::makeCatalogueEntry() {
  // create the catalogue
  s3Bucket_ = "hermes-databucket-aws";
  filesystem::path metadataPath = std::filesystem::current_path()
                                      .parent_path()
                                      .append("resources/metadata");
  catalogue_ = make_shared<Catalogue>("main", metadataPath);

  // read catalogue entry
  catalogueEntry_ = S3CatalogueEntryReader::readS3CatalogueEntry(catalogue_,
                                                                 s3Bucket_,
                                                                 schemaName_,
                                                                 awsClient_->getS3Client());

  catalogue_->putEntry(catalogueEntry_);
}

void TestUtil::makeCalciteClient() {
  auto calciteConfig = CalciteConfig::parseCalciteConfig();
  calciteClient_ = make_shared<CalciteClient>(calciteConfig);
  calciteClient_->startClient();
}

void TestUtil::connect() {
  if (!actorSystemConfig_->nodeIps_.empty()) {
    for (const auto &nodeIp : actorSystemConfig_->nodeIps_) {
      auto expectedNode = actorSystem_->middleman().connect(nodeIp, actorSystemConfig_->port_);
      if (!expectedNode) {
        nodes_.clear();
        throw runtime_error(
            fmt::format("Failed to connected to server {}: {}", nodeIp, to_string(expectedNode.error())));
      }
      nodes_.emplace_back(*expectedNode);
    }
  }
}

void TestUtil::makeExecutor() {
  // create the actor system
  const auto &remoteIps = readRemoteIps();
  int CAFServerPort = ExecConfig::parseCAFServerPort();
  actorSystemConfig_ = make_shared<ActorSystemConfig>(CAFServerPort, remoteIps, false);
  CAFInit::initCAFGlobalMetaObjects();
  actorSystem_ = make_shared<::caf::actor_system>(*actorSystemConfig_);

  // create the executor
  if (isDistributed_) {
    connect();
  }
  // Get all tables from catalogue
  vector<shared_ptr<Table>> allTables(0);
  if (catalogueEntry_->getType() == CatalogueEntryType::S3) {
    allTables = catalogueEntry_->getTables();
  }
  executor_ = make_shared<Executor>(actorSystem_,
                                    nodes_,
                                    mode_,
                                    cachingPolicy_,
                                    allTables,// get all the tables
                                    true,
                                    false);
  executor_->start();
}

void TestUtil::executeQueryFile(const string &queryFileName, int qcount) {
  ////auto startTime = chrono::steady_clock::now();
  ////auto duration = duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startTime).count();
  ////SPDLOG_CRITICAL("Duration: {} sec", duration*1e-3);
  SPDLOG_CRITICAL("Query: {} num: {}", queryFileName, qcount+1);

  // Plan query
  string queryPath = std::filesystem::current_path()
                         .parent_path()
                         .append("resources/query")
                         .append(queryFileName)
                         .string();
  string query = readFile(queryPath);
  string planResult = calciteClient_->planQuery(query, schemaName_);

  // deserialize plan json string into prephysical plan
  auto planDeserializer = make_shared<CalcitePlanJsonDeserializer>(planResult, catalogueEntry_);
  const auto &prePhysicalPlan = planDeserializer->deserialize();

  // trim unused fields (Calcite trimmer does not trim completely)
  prePhysicalPlan->populateAndTrimProjectColumns();

  // transform prephysical plan to physical plan
  int numNodes = isDistributed_ ? (int) nodes_.size() : 1;
  auto prePToPTransformer = make_shared<PrePToPTransformer>(prePhysicalPlan,
                                                            awsClient_,
                                                            mode_,
                                                            parallelDegree_,
                                                            numNodes);
  const auto &physicalPlan = prePToPTransformer->transform();

  // execute
  const auto &execRes = executor_->execute(physicalPlan, isDistributed_);

  // show output
  stringstream ss;
  ///////ss << fmt::format("Result |\n{}", execRes.first->showString(TupleSetShowOptions(TupleSetShowOrientation::RowOriented, 100)));
  ss << fmt::format("\nTime: {} secs", (double) (execRes.second) / 1000000000.0);
  ss << endl;
  cout << ss.str() << endl;
}


bool TestUtil::condition(int64_t queryCounter) {
  hermes->g_m_.lock();
  bool result = hermes->getRunningQuery()!=queryCounter;
  if(result){
    hermes->setRunningQuery(queryCounter);
    hermes->setDone(false);
    std::queue<shared_ptr<arrow::Table>> empty;
    std::swap( lo_batchQueue, empty );
    std::swap( c_batchQueue, empty );
    std::swap( s_batchQueue, empty );
    std::swap( c_batchQueue, empty );
    std::swap( d_batchQueue, empty );
    for(int i=0; i<5; i++){
      hermes->batchesSeen_[i] = 0;
    }
  }
  hermes->g_m_.unlock();
  return result;
}

std::vector<std::int8_t> TestUtil::execute(const string& query, const int64_t queryCounter, string tableName, const int64_t threadCnt) {
  std::vector<int8_t> tableBytes;
  std::vector<int8_t> emptyTableBytes;
  if(condition(queryCounter)) {
    std::cout << "queryCounter: " << queryCounter  << "---->" << query << std::endl;
    if (queryCounter >= 4) {
      flag = true;
    }
    // save the result and send it to DuckDB in chunks
    result_ = executeQuery(query);
    hermes->setDone(true);
    shared_ptr<arrow::Table> currentTable;
    if (tableName == "lineorder") {
      if (!lo_batchQueue.empty()) {
        lo_mtx_.lock();
        currentTable = lo_batchQueue.front();
        lo_batchQueue.pop();
        lo_mtx_.unlock();
        tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
        SPDLOG_CRITICAL("[Main-Lineorder: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      } else {
        return emptyTableBytes;
      }
    } else if (tableName == "customer") {
      if (!c_batchQueue.empty()) {
        c_mtx_.lock();
        hermes->batchesSeen_[1]++;
        currentTable = c_batchQueue.front();
        c_batchQueue.pop();
        c_mtx_.unlock();
        tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
        SPDLOG_CRITICAL("[Main-Customer: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      } else {
        return emptyTableBytes;
      }
    } else if (tableName == "supplier") {
      if (!s_batchQueue.empty()) {
        s_mtx_.lock();
        hermes->batchesSeen_[2]++;
        currentTable = s_batchQueue.front();
        s_batchQueue.pop();
        s_mtx_.unlock();
        tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
        SPDLOG_CRITICAL("[Main-Supplier: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      } else {
        return emptyTableBytes;
      }
    } else if (tableName == "part") {
      if (!p_batchQueue.empty()) {
        p_mtx_.lock();
        hermes->batchesSeen_[3]++;
        currentTable = p_batchQueue.front();
        p_batchQueue.pop();
        p_mtx_.unlock();
        tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
        SPDLOG_CRITICAL("[Main-Part: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      } else {
        return emptyTableBytes;
      }
    } else {
      if (!d_batchQueue.empty()) {
        d_mtx_.lock();
        hermes->batchesSeen_[4]++;
        currentTable = d_batchQueue.front();
        d_batchQueue.pop();
        d_mtx_.unlock();
        tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
        SPDLOG_CRITICAL("[Main-Date: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      } else {
        return emptyTableBytes;
      }
    }
    ////lo_vBatchReady.notify_all();
  }
  else {
    if (tableName == "lineorder") {
      if(hermes->getDone() and lo_batchQueue.empty()){
        SPDLOG_CRITICAL("Empty lineorder");
        return emptyTableBytes;
      }
      std::unique_lock<std::mutex> lock(lo_mtx_);
      lo_vBatchReady.wait(lock, [&] { return (!lo_batchQueue.empty()); });//threadCnt<=gBatchCnt
      shared_ptr<arrow::Table> currentTable;
      currentTable = lo_batchQueue.front();
      lo_batchQueue.pop();
      lock.unlock();
      SPDLOG_CRITICAL("[Thread-Lineorder: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
    } else if (tableName == "customer") {
      if(hermes->batchesSeen_[1]==3 and c_batchQueue.empty()){  //15
        SPDLOG_CRITICAL("Empty customer");
        return emptyTableBytes;
      }
      std::unique_lock<std::mutex> lock(c_mtx_);
      c_vBatchReady.wait(lock, [&] { return (!c_batchQueue.empty()); });//threadCnt<=gBatchCnt
      shared_ptr<arrow::Table> currentTable;
      hermes->batchesSeen_[1]++;
      currentTable = c_batchQueue.front();
      c_batchQueue.pop();
      lock.unlock();
      SPDLOG_CRITICAL("[Thread-Customer: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
    } else if (tableName == "supplier") {
      if(hermes->batchesSeen_[2]==1 and s_batchQueue.empty()){
        SPDLOG_CRITICAL("Empty supplier");
        return emptyTableBytes;
      }
      std::unique_lock<std::mutex> lock(s_mtx_);
      s_vBatchReady.wait(lock, [&] { return (!s_batchQueue.empty()); });//threadCnt<=gBatchCnt
      shared_ptr<arrow::Table> currentTable;
      hermes->batchesSeen_[2]++;
      currentTable = s_batchQueue.front();
      s_batchQueue.pop();
      lock.unlock();
      SPDLOG_CRITICAL("[Thread-Supplier: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
    } else if (tableName == "part") {
      if(hermes->batchesSeen_[3]==8 and p_batchQueue.empty()){ //12
        SPDLOG_CRITICAL("Empty part");
        return emptyTableBytes;
      }
      std::unique_lock<std::mutex> lock(p_mtx_);
      p_vBatchReady.wait(lock, [&] { return (!p_batchQueue.empty()); });//threadCnt<=gBatchCnt
      shared_ptr<arrow::Table> currentTable;
      hermes->batchesSeen_[3]++;
      currentTable = p_batchQueue.front();
      p_batchQueue.pop();
      lock.unlock();
      SPDLOG_CRITICAL("[Thread-Part: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
    } else {
      SPDLOG_CRITICAL("Request for {}--> {}:{}", tableName, hermes->batchesSeen_[4], d_batchQueue.empty());
      if(hermes->batchesSeen_[4]==1 and d_batchQueue.empty()){
        SPDLOG_CRITICAL("Empty date");
        return emptyTableBytes;
      }
      std::unique_lock<std::mutex> lock(d_mtx_);
      d_vBatchReady.wait(lock, [&] { return (!d_batchQueue.empty()); });//threadCnt<=gBatchCnt
      shared_ptr<arrow::Table> currentTable;
      hermes->batchesSeen_[4]++;
      currentTable = d_batchQueue.front();
      d_batchQueue.pop();
      lock.unlock();
      SPDLOG_CRITICAL("[Thread-Date: {}] Size of batch: {}", threadCnt, currentTable->num_rows());
      tableBytes = arrowTableToParquet(currentTable); ///ArrowSerializer::table_to_bytes2(currentTable);
    }
  }
  return tableBytes;
}

std::shared_ptr<TupleSet> TestUtil::executeQuery(const string& query){
  //SPDLOG_CRITICAL("Query: {} started", query);
  string planResult = calciteClient_->planQuery(query, schemaName_);
  auto planDeserializer = make_shared<CalcitePlanJsonDeserializer>(planResult, catalogueEntry_);
  const auto &prePhysicalPlan = planDeserializer->deserialize();
  prePhysicalPlan->populateAndTrimProjectColumns();
  int numNodes = isDistributed_ ? (int) nodes_.size() : 1;
  auto prePToPTransformer = make_shared<PrePToPTransformer>(prePhysicalPlan,
                                                            awsClient_,
                                                            mode_,
                                                            parallelDegree_,
                                                            numNodes);
  const auto &physicalPlan = prePToPTransformer->transform();
  const auto &execRes = executor_->execute(physicalPlan, isDistributed_);
  // show output
  /*stringstream ss;
  ss << fmt::format("Result |\n{}", execRes.first->showString(TupleSetShowOptions(TupleSetShowOrientation::RowOriented, 100)));
  ss << fmt::format("\nTime: {} secs", (double) (execRes.second) / 1000000000.0);
  ss << endl;
  cout << ss.str() << endl;*/
  SPDLOG_CRITICAL("Total execution time of query: {} secs",(double) (execRes.second) / 1000000000.0);
  return execRes.first;
}

std::vector<int8_t> TestUtil::arrowTableToParquet(shared_ptr<arrow::Table> arrowTable){
  auto writerProperties = ::parquet::WriterProperties::Builder()
                              .max_row_group_length(fpdb::tuple::DefaultChunkSize)
                              ->compression(::parquet::Compression::type::SNAPPY)
                              ->build();
  auto arrowWriterProperties = ::parquet::ArrowWriterProperties::Builder().store_schema()->build();
  auto outputBuffer = arrow::io::BufferOutputStream::Create(0, arrow::default_memory_pool());
  auto result = ::parquet::arrow::WriteTable(*(arrowTable),
                                             arrow::default_memory_pool(),
                                             *outputBuffer,
                                             DefaultChunkSize,
                                             writerProperties,
                                             arrowWriterProperties);

  auto parquetBuffer = (*(*outputBuffer)->Finish());
  auto parquetBufferData = parquetBuffer->data();
  auto bufferSize = parquetBuffer->size();
  std::vector<int8_t> tableBytes;
  for(int j=0; j<bufferSize; j++){
    tableBytes.push_back(*(parquetBufferData+j));
  }
  (*outputBuffer)->Close();
  return tableBytes;
}

bool TestUtil::runTime(chrono::steady_clock::time_point &startTime, int duration) {
  if (duration_cast<seconds>(steady_clock::now() - startTime).count() <= duration and duration - duration_cast<seconds>(steady_clock::now() - startTime).count() > 30)
    return true;
  else
    return false;
}

}
