//
// Created by matt on 12/8/20.
//

#ifndef FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_PARQUETREADER_H
#define FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_PARQUETREADER_H

#include <string>
#include <memory>

#include <tl/expected.hpp>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>

#include <fpdb/tuple/FileReader.h>
#include <fpdb/tuple/TupleSet.h>

namespace fpdb::tuple {

class ParquetReader : public FileReader {

public:
  explicit ParquetReader(std::string Path);
  ParquetReader() = default;
  ParquetReader(const ParquetReader&) = default;
  ParquetReader& operator=(const ParquetReader&) = default;
  ~ParquetReader() = default;

  static tl::expected<std::shared_ptr<ParquetReader>, std::string> make(const std::string &Path);

  tl::expected<std::shared_ptr<TupleSet>, std::string>
  read(const std::vector<std::string> &columnNames, int64_t startPos, int64_t finishPos) override;

private:
  tl::expected<void, std::string> close(const std::shared_ptr<arrow::io::ReadableFile> &inputStream);

  std::string path_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, ParquetReader& reader) {
    return f.object(reader).fields(f.field("type", reader.type_),
                                   f.field("path", reader.path_));
  }
};

}

#endif //FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_PARQUETREADER_H
