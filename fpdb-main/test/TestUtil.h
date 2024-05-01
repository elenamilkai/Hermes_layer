//
// Created by Yifei Yang on 11/30/21.
//

#ifndef FPDB_FPDB_MAIN_TEST_TESTUTIL_H
#define FPDB_FPDB_MAIN_TEST_TESTUTIL_H

#include "../../fpdb-main/include/fpdb/main/ActorSystemConfig.h"
#include <fpdb/executor/Executor.h>
#include <fpdb/catalogue/Catalogue.h>
#include <fpdb/catalogue/CatalogueEntry.h>
#include <fpdb/catalogue/s3/S3CatalogueEntry.h>
#include <fpdb/calcite/CalciteClient.h>
#include <fpdb/aws/AWSClient.h>
#include <memory>
#include <vector>
#include <string>

using namespace fpdb::main;
using namespace fpdb::executor;
using namespace fpdb::plan;
using namespace fpdb::cache;
using namespace fpdb::catalogue;
using namespace fpdb::catalogue::s3;
using namespace fpdb::calcite;
using namespace fpdb::aws;
using namespace std;

namespace fpdb::main::test {

class TestUtil {

public:
  /**
   * Test with calcite server already started, using pullup by default
   * @param schemaName
   * @param queryFileNames
   * @param parallelDegree
   * @param isDistributed
   *
   * @return whether executed successfully
   */
  static bool e2eNoStartCalciteServer(const string &schemaName,
                                      const vector<string> &queryFileNames,
                                      int parallelDegree,
                                      bool isDistributed);

  TestUtil(const string &schemaName,
           const vector<string> &queryFileNames,
           int parallelDegree,
           bool isDistributed);

public:
  void runTest();
  void makeAWSClient();
  void makeCatalogueEntry();
  void makeCalciteClient();
  void connect();
  void makeExecutor();
  void executeQueryFile(const string &queryFileName, int qcount);
  std::vector<std::int8_t> execute(const string& query, const int64_t queryCounter, string tableName, const int64_t threadCnt);
  std::vector<int8_t> arrowTableToParquet(shared_ptr<arrow::Table> arrowTable);
  std::shared_ptr<TupleSet> executeQuery(const string& query);
  bool condition(int64_t queryCounter);


  void stop();
  bool runTime(chrono::steady_clock::time_point& startTime, int duration);
private:
  // input parameters
  std::string s3Bucket_;
  unordered_map<string, vector<int>> tablesToPartitions_;
  std::string schemaName_;
  vector<string> queryFileNames_;
  int parallelDegree_;
  bool isDistributed_;

  // internal parameters
  shared_ptr<AWSClient> awsClient_;
  shared_ptr<Catalogue> catalogue_;
  shared_ptr<CatalogueEntry> catalogueEntry_;
  shared_ptr<S3CatalogueEntry> catalogueEntryS3_;
  shared_ptr<CalciteClient> calciteClient_;
  shared_ptr<ActorSystemConfig> actorSystemConfig_;
  shared_ptr<::caf::actor_system> actorSystem_;
  vector<::caf::node_id> nodes_;
  shared_ptr<Executor> executor_;
  shared_ptr<Mode> mode_;
  shared_ptr<CachingPolicy> cachingPolicy_;
  shared_ptr<TupleSet> result_;
  int chunkSize_ = 8192;

};

}

#endif //FPDB_FPDB_MAIN_TEST_TESTUTIL_H
