//
// Created by Elena Milkai on 1/1/23.
//
#ifndef FPDB_HERMESSERVERHANDLER_H
#define FPDB_HERMESSERVERHANDLER_H

#include <../gen-cpp/HermesDuckDBServer.h>
#include <../gen-cpp/HermesDuckDBServer_types.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "../../fpdb-main/test/TestUtil.h"

using namespace std;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::com::thrift::hermesduckdbserver;
using namespace fpdb::main;
using namespace fpdb::main::test;

namespace fpdb::hermesserver {
class HermesServerHandler : virtual public HermesDuckDBServerIf {
  public:
    explicit HermesServerHandler();
    void getFreshDataHermes(std::vector<int8_t>& _return,
                            const std::string& query,
                            const int64_t queryCounter,
                            const std::string& tableName);

    void printHelloHermes();
    shared_ptr<TServer> &getHermesServer();
    shared_ptr<TServerTransport> &getHermesTransport();
    shared_ptr<TestUtil> &getTestUtil();
    void shutDownServer();
  private:
    shared_ptr<TServerTransport> hermesTransport_;
    shared_ptr<TServer> hermesServer_;
    shared_ptr<TestUtil> testUtil_;
    std::atomic<int> threadCnt_;
    std::atomic<int> currentQuery_;
  };
}

#endif//FPDB_HERMESSERVERHANDLER_H
