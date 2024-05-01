//
// Created by matt on 12/8/20.
//

#include "fpdb/tuple/FileReaderBuilder.h"

using namespace fpdb::tuple;

std::shared_ptr<FileReader> FileReaderBuilder::make(const std::string &path, FileType fileType) {
  switch (fileType) {
  case FileType::CSV: return CSVReader::make(path).value();
  case FileType::Parquet: return ParquetReader::make(path).value();
  }
}
