package com.flexpushdowndb.logparser.server;
import com.thrift.logparserserver.LogParserServer;
import com.flexpushdowndb.logparser.parser.Parser;
import com.thrift.logparserserver.Binlog;
import org.apache.thrift.TException;
import org.apache.thrift.server.TServer;
import org.apache.thrift.transport.TServerTransport;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.*;
import java.util.stream.Collectors;
import org.apache.commons.lang3.ArrayUtils;
import java.util.List;

public class LogParserServerHandler implements LogParserServer.Iface{
  private final Parser parser;
  private TServerTransport serverTransport;
  private TServer server;

  public LogParserServerHandler(Path resourcePath) {
    this.parser = new Parser();
    System.out.println("Parser object is ready!");
  }

  public void setServerTransport(TServerTransport serverTransport) {
    this.serverTransport = serverTransport;
  }

  public void setServer(TServer server) {
    this.server = server;
  }

  public void ping() throws TException {
    System.out.println("[Java] LogParser client ping");
  }

  public void shutdown() throws TException{
    server.stop();
    serverTransport.close();
    System.out.println("[Java] LogParser server shutdown...");
  }

  @Override
  public void printHello() throws TException {
    System.out.println("[Java] Hello there!");

  }

  public Binlog parser() throws TException {
    Binlog binlog = new Binlog();
    byte[][] binLogs = null;
    try {
        binLogs = this.parser.parseBinlogFile();
    } catch (Exception e)  {
        e.printStackTrace();
    }

    if (binLogs != null) {
        List<List<Byte>> listall = new ArrayList<>(binLogs.length);
        for (byte[] binLog : binLogs) {
            List<Byte> list = new ArrayList<>(binLog.length);
            for (byte b : binLog) {
                list.add(b);
            }
            listall.add(list);
        }
        binlog.logs = listall;
    }

    return binlog;
  }

}
