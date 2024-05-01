//
// Created by Yifei Yang on 11/11/21.
//

#ifndef FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_FORMAT_H
#define FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_FORMAT_H

#include <fpdb/catalogue/format/FormatType.h>

namespace fpdb::catalogue::format {

class Format {
public:
  Format(FormatType type);
  Format() = default;
  Format(const Format&) = default;
  Format& operator=(const Format&) = default;

  FormatType getType() const;

protected:
  FormatType type_;
};

}


#endif //FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_FORMAT_H
