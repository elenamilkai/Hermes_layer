//
// Created by Elena Milkai on 1/9/23.
//
#ifndef FPDB_SERVER_H
#define FPDB_SERVER_H

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <fpdb/hermesserver/HermesServerHandler.h>
#include "../../fpdb-main/test/TestUtil.h"

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace fpdb::main;
using namespace fpdb::main::test;

namespace fpdb::hermesserver {
class Server {
public:
  explicit Server(shared_ptr<main::test::TestUtil> &testUtil);
  void start();
  int getRunningQuery();
  void setRunningQuery(int value);
  bool getDone();
  void setDone(bool value);
  std::mutex g_m_;
  std::vector<int> batchesSeen_;

private:
  shared_ptr<TestUtil> testUtil_;
  std::atomic<bool> done_; // indicates if the query execution is done
  std::atomic<int> runningQuery_ = 0;

};

}
#endif//FPDB_SERVER_H

