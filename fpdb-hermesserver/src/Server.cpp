//
// Created by Elena Milkai on 1/9/23.
//
#include <fpdb/hermesserver/Server.h>
#include <fmt/format.h>
#include <string>
#include <filesystem>
#include <thread>
#include <iostream>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/server/TThreadPoolServer.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::concurrency;

namespace fpdb::hermesserver {
  Server::Server(shared_ptr<fpdb::main::test::TestUtil> &testUtil) {
    testUtil_ = testUtil;
    done_ = false;
    batchesSeen_ = {5, 0};
  }

  void Server::start(){
    int serverPort = 9090;
    std::cout << "[Hermes] Server starts..." << std::endl;
    shared_ptr<HermesServerHandler> handler(new HermesServerHandler());
    shared_ptr<TProcessor> processor(new HermesDuckDBServerProcessor(handler));
    handler->getHermesTransport() = make_shared<TServerSocket>(serverPort);
    handler->getTestUtil() = testUtil_;

    // TThreadPool Server
    std::shared_ptr<ThreadFactory> threadFactory
        = std::shared_ptr<ThreadFactory>(new ThreadFactory());

    std::shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(50,50);
    threadManager->threadFactory(threadFactory);
    threadManager->start();
    handler->getHermesServer() = make_shared<TThreadPoolServer>(processor,
                                       handler->getHermesTransport(),
                                       std::make_shared<TBufferedTransportFactory>(),
                                       std::make_shared<TBinaryProtocolFactory>(),
                                       threadManager);
    handler->getHermesServer()->serve();

  }

  int Server::getRunningQuery(){
    return runningQuery_.load();
  }

  void Server::setRunningQuery(int value){
    runningQuery_ = value;
  }


  bool Server::getDone(){
    return done_.load();
  }

  void Server::setDone(bool value){
    done_ = value;
  }
}

