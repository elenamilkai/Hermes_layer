//
// Created by matt on 12/8/20.
//

#include <fpdb/tuple/FileReader.h>

namespace fpdb::tuple {

FileReader::FileReader(FileType type):
  type_(type) {}

FileType FileReader::getType() const {
  return type_;
}

}
