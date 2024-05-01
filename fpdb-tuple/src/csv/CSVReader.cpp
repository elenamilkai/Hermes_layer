//
// Created by matt on 12/8/20.
//

#include <fpdb/tuple/csv/CSVReader.h>
#include <fpdb/tuple/csv/CSVParser.h>
#include <filesystem>

using namespace fpdb::tuple;
using namespace fpdb::tuple::csv;

CSVReader::CSVReader(std::string path) :
  FileReader(FileType::CSV),
  path_(std::move(path)) {}

tl::expected<std::shared_ptr<CSVReader>, std::string> CSVReader::make(const std::string &path) {
  auto absolutePath = std::filesystem::absolute(path);
  return std::make_shared<CSVReader>(absolutePath);
}

tl::expected<std::shared_ptr<TupleSet>, std::string>
CSVReader::read(const std::vector<std::string> &columnNames, int64_t startPos, int64_t finishPos) {
  CSVParser parser(path_, columnNames, startPos, finishPos);
  return parser.parse();
}
