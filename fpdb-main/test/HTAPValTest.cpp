//
// Created by ZhangOscar on 2/22/22.
//

#include <doctest/doctest.h>
#include "TestUtil.h"

namespace fpdb::main::test {

#define SKIP_SUITE false
#define SCHEMA_PATH "ssb-sf1-htap/csv/"
#define PARALLEL_DEGREE 1

TEST_SUITE ("ssb-sf1-htap-validation" * doctest::skip(SKIP_SUITE)) {

TEST_CASE ("ssb-sf1-htap-validation" * doctest::skip(false || SKIP_SUITE)) {
      REQUIRE(TestUtil::e2eNoStartCalciteServer(SCHEMA_PATH,
                                                {"ssb/original/1.1.sql",
                                                "ssb/original/1.2.sql"},
                                                PARALLEL_DEGREE,
                                                false));

}

}
}