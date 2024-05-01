//
// Created by Yifei Yang on 11/9/21.
//

#include <fpdb/catalogue/s3/S3CatalogueEntryReader.h>
#include <fpdb/catalogue/format/CSVFormat.h>
#include <fpdb/catalogue/format/ParquetFormat.h>
#include <fpdb/tuple/ColumnName.h>
#include <fpdb/aws/S3Util.h>
#include <fpdb/util/Util.h>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

using namespace fpdb::aws;
using namespace fpdb::tuple;
using namespace fpdb::util;

namespace fpdb::catalogue::s3 {

shared_ptr<S3CatalogueEntry>
S3CatalogueEntryReader::readS3CatalogueEntry(const shared_ptr<Catalogue> &catalogue,
                                             const string &s3Bucket,
                                             const string &schemaName,
                                             const shared_ptr<S3Client> &s3Client) {
  // read metadata files
  filesystem::path metadataPath = catalogue->getMetadataPath().append(schemaName);
  if (!filesystem::exists(metadataPath)) {
    throw runtime_error(fmt::format("Metadata not found for schema: {}", schemaName));
  }
  auto schemaJObj = json::parse(readFile(metadataPath.append("schema.json")));
  string schema = to_string(schemaJObj);
  metadataPath = metadataPath.parent_path();
  auto statsJObj = json::parse(readFile(metadataPath.append("stats.json")));
  metadataPath = metadataPath.parent_path();
  auto zoneMapJObj = json::parse(readFile(metadataPath.append("zoneMap.json")));

  // get all table names
  unordered_set<string> tableNames;
  for (const auto &tableSchemaJObj: schemaJObj["tables"].get<vector<json>>()) {
    tableNames.emplace(tableSchemaJObj["name"].get<string>());
  }

  // member variables to make S3CatalogueEntry and S3Table
  unordered_map<string, shared_ptr<format::Format>> formatMap;
  unordered_map<string, shared_ptr<arrow::Schema>> schemaMap;
  unordered_map<string, unordered_map<string, int>> apxColumnLengthMapMap;
  unordered_map<string, int> apxRowLengthMap;
  unordered_map<string, unordered_set<string>> zoneMapColumnNamesMap;
  unordered_map<string, vector<shared_ptr<S3Partition>>> s3PartitionsMap;
  unordered_map<string, vector<string>> primaryKeyNamesMap;

  // read schema
  readSchema(schemaJObj, s3Bucket, schemaName, schemaMap, formatMap, primaryKeyNamesMap, s3PartitionsMap);

  // read stats
  readStats(statsJObj, apxColumnLengthMapMap, apxRowLengthMap);

  // read zoneMap
  readZoneMap(zoneMapJObj, schemaMap, s3PartitionsMap, zoneMapColumnNamesMap);

  // read partition size from s3 listObject
  readPartitionSize(s3Client, s3Bucket, schemaName, s3PartitionsMap);

  // create an S3CatalogueEntry
  shared_ptr<S3CatalogueEntry> s3CatalogueEntry = make_shared<S3CatalogueEntry>(schemaName,
                                                                                s3Bucket,
                                                                                catalogue);

  // make S3Tables
  for (const auto &tableName: tableNames) {
    shared_ptr<S3Table> s3Table = make_shared<S3Table>(tableName,
                                                       primaryKeyNamesMap.find(tableName)->second,
                                                       schemaMap.find(tableName)->second,
                                                       formatMap.find(tableName)->second,
                                                       apxColumnLengthMapMap.find(tableName)->second,
                                                       apxRowLengthMap.find(tableName)->second,
                                                       zoneMapColumnNamesMap.find(tableName)->second,
                                                       s3PartitionsMap.find(tableName)->second);
    s3CatalogueEntry->addS3Table(s3Table);
  }
  return s3CatalogueEntry;
}

void S3CatalogueEntryReader::readSchema(const json &schemaJObj,
                                        const string &s3Bucket,
                                        const string &schemaName,
                                        unordered_map<string, shared_ptr<arrow::Schema>> &schemaMap,
                                        unordered_map<string, shared_ptr<format::Format>> &formatMap,
                                        unordered_map<string, vector<string>> &primaryKeyNamesMap,
                                        unordered_map<string, vector<shared_ptr<S3Partition>>> &s3PartitionsMap) {
  // read schemas, formats and s3Partitions
  for (const auto &tableSchemasJObj: schemaJObj["tables"].get<vector<json>>()) {
    const string &tableName = tableSchemasJObj["name"].get<string>();

    // formats
    const auto &formatJObj = tableSchemasJObj["format"];
    const string &formatStr = formatJObj["name"].get<string>();
    string s3ObjectSuffix;
    if (formatStr == "csv") {
      char fieldDelimiter = formatJObj["fieldDelimiter"].get<string>().c_str()[0];
      formatMap.emplace(tableName, make_shared<format::CSVFormat>(fieldDelimiter));
      s3ObjectSuffix = ".tbl";
    } else if (formatStr == "parquet") {
      formatMap.emplace(tableName, make_shared<format::ParquetFormat>());
      s3ObjectSuffix = ".parquet";
    } else {
      throw runtime_error(fmt::format("Unsupported data format: {}", formatStr));
    }

    // primary key column names
    vector<string> primaryKeyNames = ColumnName::canonicalize(tableSchemasJObj["primaryKeyNames"].get<vector<string>>());
    primaryKeyNamesMap.emplace(tableName, primaryKeyNames);

    // fields
    vector<shared_ptr<arrow::Field>> fields;
    for (const auto &fieldJObj: tableSchemasJObj["fields"].get<vector<json>>()) {
      const string &fieldName = ColumnName::canonicalize(fieldJObj["name"].get<string>());
      const string &fieldTypeStr = fieldJObj["type"].get<string>();
      const auto &fieldType = strToDataType(fieldTypeStr);
      auto field = make_shared<arrow::Field>(fieldName, fieldType);
      fields.emplace_back(field);
    }
    auto schema = make_shared<arrow::Schema>(fields);
    schemaMap.emplace(tableName, schema);

    // partitions
    vector<shared_ptr<S3Partition>> s3Partitions;
    int numPartitions = tableSchemasJObj["numPartitions"].get<int>();
    if (numPartitions == 1) {
      string s3Object = fmt::format("{}{}{}", schemaName, tableName, s3ObjectSuffix);
      s3Partitions.emplace_back(make_shared<S3Partition>(s3Bucket, s3Object));
    } else {
      string s3ObjectDir = schemaName + tableName + "_sharded/";
      for (int i = 0; i < numPartitions; ++i) {
        string s3Object = fmt::format("{}{}{}.{}", s3ObjectDir, tableName, s3ObjectSuffix, to_string(i));
        s3Partitions.emplace_back(make_shared<S3Partition>(s3Bucket, s3Object));
      }
    }
    s3PartitionsMap.emplace(tableName, s3Partitions);
  }
}

void S3CatalogueEntryReader::readStats(const json &statsJObj,
                                       unordered_map<string, unordered_map<string, int>> &apxColumnLengthMapMap,
                                       unordered_map<string, int> &apxRowLengthMap) {
  for (const auto &tableStatsJObj: statsJObj["tables"].get<vector<json>>()) {
    const string &tableName = tableStatsJObj["name"].get<string>();
    unordered_map<string, int> apxColumnLengthMap;
    int apxRowLength = 0;
    for (const auto &apxColumnLengthIt: tableStatsJObj["stats"]["apxColumnLength"].get<unordered_map<string, int>>()) {
      string columnName = ColumnName::canonicalize(apxColumnLengthIt.first);
      int apxColumnLength = apxColumnLengthIt.second;
      apxColumnLengthMap.emplace(columnName, apxColumnLength);
      apxRowLength += apxColumnLength;
    }
    apxColumnLengthMapMap.emplace(tableName, apxColumnLengthMap);
    apxRowLengthMap.emplace(tableName, apxRowLength);
  }
}

void S3CatalogueEntryReader::readZoneMap(const json &zoneMapJObj,
                                         const unordered_map<string, shared_ptr<arrow::Schema>> &schemaMap,
                                         unordered_map<string, vector<shared_ptr<S3Partition>>> &s3PartitionsMap,
                                         unordered_map<string, unordered_set<string>> &zoneMapColumnNamesMap) {
  // read zoneMaps
  for (const auto &tableZoneMapsJObj: zoneMapJObj["tables"].get<vector<json>>()) {
    const string &tableName = tableZoneMapsJObj["name"].get<string>();
    const shared_ptr<arrow::Schema> &schema = schemaMap.find(tableName)->second;
    const vector<shared_ptr<S3Partition>> &s3Partitions = s3PartitionsMap.find(tableName)->second;

    unordered_set<string> zoneMapColumnNames;
    for (const auto &tableZoneMapJObj: tableZoneMapsJObj["zoneMap"].get<vector<json>>()) {
      const string &fieldName = ColumnName::canonicalize(tableZoneMapJObj["field"].get<string>());
      zoneMapColumnNames.emplace(fieldName);
      const shared_ptr<arrow::DataType> fieldType = schema->GetFieldByName(fieldName)->type();
      const auto valuePairsJArr = tableZoneMapJObj["valuePairs"].get<vector<json>>();

      for (size_t i = 0; i < s3Partitions.size(); ++i) {
        const auto &s3Partition = s3Partitions[i];
        const auto valuePairJObj = valuePairsJArr[i];
        s3Partition->addMinMax(fieldName, jsonToMinMaxLiterals(valuePairJObj, fieldType));
      }
    }
    zoneMapColumnNamesMap.emplace(tableName, zoneMapColumnNames);
  }

  // fill zoneMapColumnNamesMap for no-zoneMap tables
  for (const auto &it: schemaMap) {
    const string &tableName = it.first;
    if (zoneMapColumnNamesMap.find(tableName) == zoneMapColumnNamesMap.end()) {
      zoneMapColumnNamesMap.emplace(tableName, unordered_set<string>{});
    }
  }
}

void S3CatalogueEntryReader::readPartitionSize(const shared_ptr<S3Client> &s3Client,
                                               const string &s3Bucket,
                                               const string &schemaName,
                                               unordered_map<string, vector<shared_ptr<S3Partition>>> &s3PartitionsMap) {
  // collect a map of s3Object -> s3Partition
  vector<shared_ptr<S3Partition>> allS3Partitions;
  for (const auto &it: s3PartitionsMap) {
    allS3Partitions.insert(allS3Partitions.end(), it.second.begin(), it.second.end());
  }
  vector<string> allS3Objects;
  unordered_map<string, shared_ptr<S3Partition>> s3ObjectPartitionMap;
  for (const auto &s3Partition: allS3Partitions) {
    const auto &s3Object = s3Partition->getObject();
    allS3Objects.emplace_back(s3Object);
    s3ObjectPartitionMap.emplace(s3Object, s3Partition);
  }

  // list objects
  auto s3ObjectSizeMap = S3Util::listObjects(s3Bucket, schemaName, allS3Objects, s3Client);

  // set partition size
  for (const auto &it: s3ObjectSizeMap) {
    const auto &s3Object = it.first;
    long objectSize = it.second;
    const auto &s3Partition = s3ObjectPartitionMap.find(s3Object)->second;
    s3Partition->setNumBytes(objectSize);
  }
}

shared_ptr<arrow::DataType> S3CatalogueEntryReader::strToDataType(const string &str) {
  if (str == "int32" || str == "int") {
    return arrow::int32();
  } else if (str == "int64" || str == "long") {
    return arrow::int64();
  } else if (str == "float64" || str == "double") {
    return arrow::float64();
  } else if (str == "utf8" || str == "string") {
    return arrow::utf8();
  } else if (str == "boolean" || str == "bool") {
    return arrow::boolean();
  } else if (str == "date") {
    return arrow::date64();
  } else {
    throw runtime_error(fmt::format("Unsupported data type: {}", str));
  }
}

pair<shared_ptr<Scalar>, shared_ptr<Scalar>>
S3CatalogueEntryReader::jsonToMinMaxLiterals(const json &jObj, const shared_ptr<arrow::DataType> &datatype) {
  if (datatype->id() == arrow::int32()->id()) {
    return make_pair(Scalar::make(arrow::MakeScalar(arrow::int32(), jObj["min"].get<int>()).ValueOrDie()),
                     Scalar::make(arrow::MakeScalar(arrow::int32(), jObj["max"].get<int>()).ValueOrDie()));
  } else if (datatype->id() == arrow::int64()->id()) {
    return make_pair(Scalar::make(arrow::MakeScalar(arrow::int64(), jObj["min"].get<long>()).ValueOrDie()),
                     Scalar::make(arrow::MakeScalar(arrow::int64(), jObj["max"].get<long>()).ValueOrDie()));
  } else if (datatype->id() == arrow::float64()->id()) {
    return make_pair(Scalar::make(arrow::MakeScalar(arrow::float64(), jObj["min"].get<double>()).ValueOrDie()),
                     Scalar::make(arrow::MakeScalar(arrow::float64(), jObj["max"].get<double>()).ValueOrDie()));
  } else if (datatype->id() == arrow::utf8()->id()) {
    return make_pair(Scalar::make(arrow::MakeScalar(arrow::utf8(), jObj["min"].get<string>()).ValueOrDie()),
                     Scalar::make(arrow::MakeScalar(arrow::utf8(), jObj["max"].get<string>()).ValueOrDie()));
  } else {
    throw runtime_error(fmt::format("Unsupported data type: {}", datatype->name()));
  }
}

}
