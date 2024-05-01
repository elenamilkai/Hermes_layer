//
// Created by Yifei Yang on 11/11/21.
//

#ifndef FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_CSVFORMAT_H
#define FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_CSVFORMAT_H

#include <fpdb/catalogue/format/Format.h>

namespace fpdb::catalogue::format {

class CSVFormat: public Format {

public:
  CSVFormat(char fieldDelimiter);
  CSVFormat() = default;
  CSVFormat(const CSVFormat&) = default;
  CSVFormat& operator=(const CSVFormat&) = default;

  char getFieldDelimiter() const;

private:
  char fieldDelimiter_;

// caf inspect
public:
  template <class Inspector>
  friend bool inspect(Inspector& f, CSVFormat& format) {
    return f.object(format).fields(f.field("type", format.type_),
                                   f.field("fieldDelimiter", format.fieldDelimiter_));
  }
};

}


#endif //FPDB_FPDB_CATALOGUE_INCLUDE_FPDB_CATALOGUE_FORMAT_CSVFORMAT_H
