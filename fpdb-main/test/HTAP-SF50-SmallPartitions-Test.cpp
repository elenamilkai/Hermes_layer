//
// Created by Elena Milkai on 5/29/23.
//

//
// Created by Elena Milkai on 5/22/23.
//
#include "TestUtil.h"
#include <doctest/doctest.h>

namespace fpdb::main::test {

#define SKIP_SUITE false
#define SCHEMA_PATH "hattrick-sf50-smallpartitions/parquet/"
#define PARALLEL_DEGREE 1

TEST_SUITE("ssb-sf50-smallpartitions-single_node-no-parallel" * doctest::skip(SKIP_SUITE)) {

  TEST_CASE("ssb-sf50-smallpartitions-single_node-no-parallel-original" * doctest::skip(false || SKIP_SUITE)) {
    REQUIRE(TestUtil::e2eNoStartCalciteServer(SCHEMA_PATH,
                                              {
                                                  "ssb/original/1.sql",
                                                  "ssb/original/1.sql",
                                                  "ssb/original/1.sql",
                                                  "ssb/original/1.sql",
                                                  "ssb/original/1.sql",
                                                  "ssb/original/1.sql"
                                                  /*"ssb/original/4.3.sql",
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
                                                  "ssb/original/4.3.sql",
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
                                                  "ssb/original/4.3.sql"*/
                                              },
                                              PARALLEL_DEGREE,
                                              false));
  }
}

}