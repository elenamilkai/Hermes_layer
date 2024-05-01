namespace java com.thrift.logparserserver

struct Binlog {
  1: list<list<byte>> logs
}

service LogParserServer {
  void ping(),
  void shutdown(),
  void printHello(),
  Binlog parser()
}