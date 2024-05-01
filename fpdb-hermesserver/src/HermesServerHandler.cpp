 //
// Created by Elena Milkai on 1/1/23.
//
#include <iostream>
#include <unistd.h>
#include <fmt/format.h>
#include <string>
#include <filesystem>
#include <thread>
#include <iostream>
#include <fpdb/hermesserver/HermesServerHandler.h>
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
using std::chrono::steady_clock;
using std::chrono::high_resolution_clock;
using namespace ::com::thrift::hermesduckdbserver;

namespace fpdb::hermesserver {
  HermesServerHandler::HermesServerHandler() {
    threadCnt_ = 0;
    currentQuery_ = 0;
  }

  void HermesServerHandler::getFreshDataHermes(std::vector<int8_t>& _return,
                                               const std::string& query,
                                               const int64_t queryCounter,
                                               const std::string& tableName) {
    if(currentQuery_.load()!=queryCounter){
      threadCnt_ = 0;
      currentQuery_ = queryCounter;
    }
    auto startTime2 = std::chrono::steady_clock::now();
    _return = testUtil_->execute(query, queryCounter, tableName, ++threadCnt_);
  }

  void HermesServerHandler::printHelloHermes() {
      SPDLOG_CRITICAL("Hello from the MySQL client!");
  }

  shared_ptr<TServer> &HermesServerHandler::getHermesServer() {
    return hermesServer_;
  }

  shared_ptr<TServerTransport> &HermesServerHandler::getHermesTransport(){
    return hermesTransport_;
  }

  shared_ptr<TestUtil> &HermesServerHandler::getTestUtil(){
    return testUtil_;
  }

  void HermesServerHandler::shutDownServer(){
    hermesServer_->stop();
    hermesTransport_->close();
  }
  }

