package com.flexpushdowndb.calcite.server;

import com.flexpushdowndb.calcite.optimizer.Optimizer;
import com.flexpushdowndb.calcite.serializer.RelJsonSerializer;
import com.thrift.calciteserver.CalciteServer;
import com.thrift.calciteserver.ParsePlanningError;
import com.thrift.calciteserver.TPlanResult;
import org.apache.calcite.rel.RelNode;
import org.apache.thrift.TException;
import org.apache.thrift.server.TServer;
import org.apache.thrift.transport.TServerTransport;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.file.Path;
import java.util.Arrays;

public class CalciteServerHandler implements CalciteServer.Iface{
  private final Optimizer optimizer;
  private TServerTransport serverTransport;
  private TServer server;

  public CalciteServerHandler(Path resourcePath) {
    this.optimizer = new Optimizer(resourcePath);
  }

  public void setServerTransport(TServerTransport serverTransport) {
    this.serverTransport = serverTransport;
  }

  public void setServer(TServer server) {
    this.server = server;
  }

  public void ping() throws TException {
    System.out.println("[Java] Client ping");
  }

  public void shutdown() throws TException{
    server.stop();
    serverTransport.close();
    System.out.println("[Java] Calcite server shutdown...");
  }

  @Override
  public TPlanResult sql2Plan(String query, String schemaName) throws TException {
    long startTime = System.currentTimeMillis();
    TPlanResult tPlanResult = new TPlanResult();
    try {
      RelNode queryPlan = optimizer.planQuery(query, schemaName);
      tPlanResult.plan_result = RelJsonSerializer.serialize(queryPlan).toString(2);
    } catch (Exception e) {
      StringWriter sw = new StringWriter();
      PrintWriter pw = new PrintWriter(sw);
      e.printStackTrace(pw);
      throw new ParsePlanningError(sw.toString());
    }
    tPlanResult.execution_time_ms = System.currentTimeMillis() - startTime;
    return tPlanResult;
  }

  @Override
  public void updateMetadata(String catalog, String table) throws TException {

  }
}
