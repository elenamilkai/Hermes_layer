//
// Created by matt on 12/8/20.
//

#include <fpdb/executor/physical/file/FileScanKernel.h>

using namespace fpdb::executor::physical::file;

FileScanKernel::FileScanKernel(std::string path,
							   std::shared_ptr<FileReader> reader,
                 int64_t startPos,
                 int64_t finishPos) :
	path_(std::move(path)),
	reader_(std::move(reader)),
	startPos_(startPos),
	finishPos_(finishPos) {}

FileScanKernel FileScanKernel::make(const std::string &path,
                                    FileType fileType,
                                    int64_t startPos,
                                    int64_t finishPos) {

  auto reader = FileReaderBuilder::make(path, fileType);

  return {path, std::move(reader), startPos, finishPos};
}

tl::expected<std::shared_ptr<TupleSet>, std::string>
FileScanKernel::scan(const std::vector<std::string> &columnNames) {
  return reader_->read(columnNames, startPos_, finishPos_);
}

const std::string &FileScanKernel::getPath() const {
  return path_;
}

FileType FileScanKernel::getFileType() const {
  return reader_->getType();
}

int64_t FileScanKernel::getStartPos() const {
  return startPos_;
}

int64_t FileScanKernel::getFinishPos() const {
  return finishPos_;
}
