//
// Created by matt on 12/8/20.
//

#ifndef FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANKERNEL_H
#define FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANKERNEL_H

#include <fpdb/tuple/FileReader.h>
#include <fpdb/tuple/FileType.h>
#include <fpdb/tuple/TupleSet.h>
#include <fpdb/tuple/FileReaderBuilder.h>
#include <tl/expected.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>

using namespace fpdb::tuple;

namespace fpdb::executor::physical::file {

class FileScanKernel {

public:
  FileScanKernel(std::string path,
				 std::shared_ptr<FileReader> reader,
				 int64_t startPos,
                 int64_t finishPos);
  FileScanKernel() = default;
  FileScanKernel(const FileScanKernel&) = default;
  FileScanKernel& operator=(const FileScanKernel&) = default;

  static FileScanKernel make(const std::string &path,
                             FileType fileType,
                             int64_t startPos,
                             int64_t finishPos);

  tl::expected<std::shared_ptr<TupleSet>, std::string> scan(const std::vector<std::string> &columnNames);

  [[nodiscard]] const std::string &getPath() const;
  [[nodiscard]] FileType getFileType() const;
  [[nodiscard]] int64_t getStartPos() const;
  [[nodiscard]] int64_t getFinishPos() const;

private:
  std::string path_;
  std::shared_ptr<FileReader> reader_;
  int64_t startPos_;
  int64_t finishPos_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, FileScanKernel& kernel) {
    return f.object(kernel).fields(f.field("path", kernel.path_),
                                   f.field("reader", kernel.reader_),
                                   f.field("startPos", kernel.startPos_),
                                   f.field("finishPos", kernel.finishPos_));
  }
};

}

#endif //FPDB_FPDB_EXECUTOR_INCLUDE_FPDB_EXECUTOR_PHYSICAL_FILE_FILESCANKERNEL_H
