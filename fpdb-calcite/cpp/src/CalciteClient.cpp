//
// Created by Yifei Yang on 10/13/21.
//
#include <fpdb/calcite/CalciteClient.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <fmt/format.h>
#include <string>
#include <filesystem>
#include <thread>
#include <spdlog/spdlog.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

namespace fpdb::calcite {
CalciteClient::CalciteClient(const shared_ptr<CalciteConfig> &calciteConfig) :
  calciteConfig_(calciteConfig) {
}

void CalciteClient::startClient() {
  shared_ptr<TSocket> socket = make_shared<TSocket>("localhost", calciteConfig_->getPort());
  shared_ptr<TTransport> transport = make_shared<TBufferedTransport>(socket);
  shared_ptr<TProtocol> protocol = make_shared<TBinaryProtocol>(transport);
  transport->open();
  calciteServerClient_ = make_shared<CalciteServerClient>(protocol);
}

void CalciteClient::startServer() {
  string calciteServerJarPath = std::filesystem::current_path()
          .parent_path()
          .parent_path()
          .append("fpdb-calcite/java/target")
          .append(calciteConfig_->getJarName())
          .string();
  string cmd = "java -jar " + calciteServerJarPath + " &";
  system(cmd.c_str());
  // Command is async, need a bit time to fully start Calcite server
  this_thread::sleep_for (std::chrono::milliseconds(2000));
}

void CalciteClient::shutdownServer() {
  calciteServerClient_->shutdown();
}

string CalciteClient::planQuery(const string &query, const string &schemaName) {
  TPlanResult planResult;
  try {
    calciteServerClient_->sql2Plan(planResult, query, schemaName);
  } catch (const ::apache::thrift::TException &e) {
    throw runtime_error(fmt::format("Calcite planning error: {}", e.what()));
  }
  return planResult.plan_result;
}
}
