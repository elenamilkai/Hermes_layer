//
// Created by matt on 12/8/20.
//

#ifndef FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_FILEREADERBUILDER_H
#define FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_FILEREADERBUILDER_H

#include <string>

#include "FileReader.h"
#include "FileType.h"
#include "fpdb/tuple/csv/CSVReader.h"
#include "fpdb/tuple/parquet/ParquetReader.h"

namespace fpdb::tuple {

class FileReaderBuilder {

public:
  static std::shared_ptr<FileReader> make(const std::string &path, FileType fileType);

};

}

#endif //FPDB_FPDB_TUPLE_INCLUDE_FPDB_TUPLE_FILEREADERBUILDER_H
