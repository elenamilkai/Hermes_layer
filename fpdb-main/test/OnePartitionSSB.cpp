//
// Created by Elena Milkai on 5/2/22.
//
#include "TestUtil.h"
#include <doctest/doctest.h>

namespace fpdb::main::test {

#define SKIP_SUITE false
#define SCHEMA_PATH "one-partition-ssb/csv/"
#define PARALLEL_DEGREE 1

TEST_SUITE("one-partition-ssb" * doctest::skip(SKIP_SUITE)) {

  TEST_CASE("one-partition-ssb-test-1" * doctest::skip(false || SKIP_SUITE)) {
    REQUIRE(TestUtil::e2eNoStartCalciteServer(SCHEMA_PATH,
                                              {"ssb/test/1.sql",
                                               "ssb/test/2.sql",
                                               "ssb/test/3.sql"},
                                              PARALLEL_DEGREE,
                                              false));
  }
}
}