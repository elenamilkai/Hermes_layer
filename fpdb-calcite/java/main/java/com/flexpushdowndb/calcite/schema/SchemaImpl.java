package com.flexpushdowndb.calcite.schema;

import org.apache.calcite.schema.Table;
import org.apache.calcite.schema.impl.AbstractSchema;

import java.util.Map;

public final class SchemaImpl extends AbstractSchema {
  private final String schemaName;
  private final Map<String, Table> tableMap;

  public SchemaImpl(String schemaName, Map<String, Table> tableMap) {
    this.schemaName = schemaName;
    this.tableMap = tableMap;
  }

  public String getSchemaName() {
    return schemaName;
  }

  @Override
  public Map<String, Table> getTableMap() {
    return tableMap;
  }
}
