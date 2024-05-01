//
// Created by Elena Milkai on 4/9/22.
//

#ifndef FPDB_LOGPARSERCLIENT_H
#define FPDB_LOGPARSERCLIENT_H

#include <../gen-cpp/LogParserServer.h>
#include <memory>
#include <vector>


namespace fpdb::logparser {

    class LogParserClient {
    public:
        explicit LogParserClient();
        void startClient();
        void startServer();
        void printHelloInJava();
        void shutdownServer();
        std::vector<std::vector<int8_t>> parser();
    private:
        std::shared_ptr<LogParserServerClient> logParserServerClient_;
    };

}
#endif //FPDB_LOGPARSERCLIENT_H
