//
// Created by Elena Milkai on 4/9/22.
//
#include <fpdb/logparser/LogParserClient.h>
#include <fmt/format.h>
#include <string>
#include <filesystem>
#include <thread>
#include <iostream>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;

namespace fpdb::logparser {
    LogParserClient::LogParserClient() {
    }

    void LogParserClient::startClient() {
        shared_ptr<TSocket> socket = make_shared<TSocket>("localhost", 9091);
        shared_ptr<TTransport> transport = make_shared<TBufferedTransport>(socket);
        shared_ptr<TProtocol> protocol = make_shared<TBinaryProtocol>(transport);
        transport->open();
        logParserServerClient_ = make_shared<LogParserServerClient>(protocol);
    }

    void LogParserClient::startServer() {
        string logParserServerJarPath =  std::filesystem::current_path()
                                          .parent_path()
                                          .append("/home/ubuntu/FPDB/fpdb-logparser/java/target/flexpushdowndb.thrift.logparser-1.0-SNAPSHOT.jar")
                                          .string();
        string cmd = "java -jar " + logParserServerJarPath + " &";
        system(cmd.c_str());
        // Command is async, need a bit time to fully start Calcite server
        this_thread::sleep_for (std::chrono::milliseconds(2000));
    }

    void LogParserClient::shutdownServer() {
        logParserServerClient_->shutdown();
    }
    void LogParserClient::printHelloInJava() {
        std::cout << "Ready to print in JAVA" << std::endl;
        try {
            logParserServerClient_->printHello();
        } catch (const ::apache::thrift::TException &e) {
            throw runtime_error(fmt::format("LogParser printing error: {}", e.what()));
        }
    }

    std::vector<std::vector<int8_t>> LogParserClient::parser() {
        Binlog binlog;
        try {
           logParserServerClient_->parser(binlog);
        } catch (const ::apache::thrift::TException &e) {
            throw runtime_error(fmt::format("LogParser parsing error: {}", e.what()));
        }
        return binlog.logs;
    }

}