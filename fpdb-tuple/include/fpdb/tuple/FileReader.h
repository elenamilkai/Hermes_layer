//
// Created by matt on 12/8/20.
//

#ifndef FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_FILEREADER_H
#define FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_FILEREADER_H

#include <fpdb/tuple/FileType.h>
#include <fpdb/tuple/TupleSet.h>
#include <tl/expected.hpp>
#include <memory>

namespace fpdb::tuple {

class FileReader {
  
public:
  FileReader(FileType type);
  FileReader() = default;
  FileReader(const FileReader&) = default;
  FileReader& operator=(const FileReader&) = default;
  virtual ~FileReader() = default;

  FileType getType() const;

  virtual tl::expected<std::shared_ptr<TupleSet>, std::string>
  read(const std::vector<std::string> &columnNames, int64_t startPos, int64_t finishPos) = 0;

protected:
  FileType type_;

};

}

#endif //FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_FILEREADER_H
