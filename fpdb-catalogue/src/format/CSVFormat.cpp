//
// Created by Yifei Yang on 11/11/21.
//

#include <fpdb/catalogue/format/CSVFormat.h>

namespace fpdb::catalogue::format {

CSVFormat::CSVFormat(char fieldDelimiter) :
  Format(CSV),
  fieldDelimiter_(fieldDelimiter) {}

char CSVFormat::getFieldDelimiter() const {
  return fieldDelimiter_;
}

}
