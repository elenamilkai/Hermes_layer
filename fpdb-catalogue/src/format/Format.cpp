//
// Created by Yifei Yang on 11/11/21.
//

#include <fpdb/catalogue/format/Format.h>

namespace fpdb::catalogue::format {

Format::Format(FormatType type) : type_(type) {}

FormatType Format::getType() const {
  return type_;
}

}
