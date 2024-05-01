//
// Created by matt on 7/8/20.
//

#ifndef FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_UNIVERSALSQLHASHFUNCTION_H
#define FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_UNIVERSALSQLHASHFUNCTION_H

#include <memory>

#include "UniversalHashFunction.h"

/**
 * Additionally defines a sql method for generating SQL predicates from the hash function
 */
class UniversalSQLHashFunction : public UniversalHashFunction {

public:
  explicit UniversalSQLHashFunction(int m);
  [[nodiscard]] static std::shared_ptr<UniversalSQLHashFunction> make(int m);

  [[nodiscard]] std::string sql(int x);

};

#endif //FPDB_FPDB_PUSHDOWN_INCLUDE_FPDB_PUSHDOWN_BLOOMJOIN_UNIVERSALSQLHASHFUNCTION_H
