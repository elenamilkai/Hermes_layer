// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "CalciteServer.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

class CalciteServerHandler : virtual public CalciteServerIf {
 public:
  CalciteServerHandler() {
    // Your initialization goes here
  }

  void ping() {
    // Your implementation goes here
    printf("ping\n");
  }

  void shutdown() {
    // Your implementation goes here
    printf("shutdown\n");
  }

  void sql2Plan(TPlanResult& _return, const std::string& query, const std::string& schemaName) {
    // Your implementation goes here
    printf("sql2Plan\n");
  }

  void updateMetadata(const std::string& catalog, const std::string& table) {
    // Your implementation goes here
    printf("updateMetadata\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  ::std::shared_ptr<CalciteServerHandler> handler(new CalciteServerHandler());
  ::std::shared_ptr<TProcessor> processor(new CalciteServerProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}
