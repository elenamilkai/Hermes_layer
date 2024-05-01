//
// Created by Yifei Yang on 11/30/21.
//

#include "TestUtil.h"
#include <doctest/doctest.h>

namespace fpdb::main::test {

#define SKIP_SUITE false
#define SCHEMA_PATH "ssb-sf1-htap/csv/"
#define PARALLEL_DEGREE 1

TEST_SUITE("ssb-sf1-single_node-no-parallel" * doctest::skip(SKIP_SUITE)) {

  TEST_CASE("ssb-sf1-single_node-no-parallel-original" * doctest::skip(false || SKIP_SUITE)) {
    REQUIRE(TestUtil::e2eNoStartCalciteServer(SCHEMA_PATH,
                                              {
                                                  "ssb/original/1.1.sql",
                                                  "ssb/original/1.2.sql",
                                                  "ssb/original/1.3.sql",
                                                  "ssb/original/2.1.sql",
                                                  "ssb/original/2.2.sql",
                                                  "ssb/original/2.3.sql",
                                                  "ssb/original/3.1.sql",
                                                  "ssb/original/3.2.sql",
                                                  "ssb/original/3.3.sql",
                                                  "ssb/original/3.4.sql",
                                                  "ssb/original/4.1.sql",
                                                  "ssb/original/4.2.sql",
                                                  "ssb/original/4.3.sql"
                                              },
                                              PARALLEL_DEGREE,
                                              false));
  }
}

}