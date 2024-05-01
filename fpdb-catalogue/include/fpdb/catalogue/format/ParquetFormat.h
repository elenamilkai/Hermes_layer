//
// Created by Yifei Yang on 11/11/21.
//

#ifndef FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_PARQUETFORMAT_H
#define FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_PARQUETFORMAT_H

#include <fpdb/catalogue/format/Format.h>

namespace fpdb::catalogue::format {

class ParquetFormat: public Format{
  
public:
  ParquetFormat();
  ParquetFormat(const ParquetFormat&) = default;
  ParquetFormat& operator=(const ParquetFormat&) = default;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, ParquetFormat& format) {
    return f.apply(format.type_);
  }
};

}


#endif //FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_PARQUETFORMAT_H
