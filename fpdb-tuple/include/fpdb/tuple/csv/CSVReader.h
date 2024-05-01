//
// Created by matt on 12/8/20.
//

#ifndef FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_CSVREADER_H
#define FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_CSVREADER_H

#include <string>
#include <memory>

#include <tl/expected.hpp>
#include <arrow/io/api.h>
#include <parquet/arrow/reader.h>

#include "fpdb/tuple/FileReader.h"
#include "fpdb/tuple/TupleSet.h"

namespace fpdb::tuple {

class CSVReader : public FileReader {

public:
  explicit CSVReader(std::string path);
  CSVReader() = default;
  CSVReader(const CSVReader&) = default;
  CSVReader& operator=(const CSVReader&) = default;

  static tl::expected<std::shared_ptr<CSVReader>, std::string> make(const std::string &path);

  [[nodiscard]] tl::expected<std::shared_ptr<TupleSet>, std::string>
  read(const std::vector<std::string> &columnNames, int64_t startPos, int64_t finishPos) override;

private:
  std::string path_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, CSVReader& reader) {
    return f.object(reader).fields(f.field("type", reader.type_),
                                   f.field("path", reader.path_));
  }
};

}

#endif //FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_CSVREADER_H
