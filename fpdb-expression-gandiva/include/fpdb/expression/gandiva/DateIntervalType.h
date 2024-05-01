//
// Created by Yifei Yang on 12/10/21.
//

#ifndef FPDB_FPDB_EXPRESSION_GANDIVA_INCLUDE_FPDB_EXPRESSION_GANDIVA_DATEINTERVALTYPE_H
#define FPDB_FPDB_EXPRESSION_GANDIVA_INCLUDE_FPDB_EXPRESSION_GANDIVA_DATEINTERVALTYPE_H

#include <fmt/format.h>

using namespace std;

namespace fpdb::expression::gandiva {

enum DateIntervalType {
  DAY,
  MONTH,
  YEAR
};

inline string intervalTypeToString(DateIntervalType intervalType) {
  switch (intervalType) {
    case DAY: {
      return "Day";
    }
    case MONTH: {
      return "Month";
    }
    case YEAR: {
      return "Year";
    }
    default:
      throw runtime_error(fmt::format("Unsupported date interval type: {}", intervalType));
  }
}

}

#endif //FPDB_FPDB_EXPRESSION_GANDIVA_INCLUDE_FPDB_EXPRESSION_GANDIVA_DATEINTERVALTYPE_H
