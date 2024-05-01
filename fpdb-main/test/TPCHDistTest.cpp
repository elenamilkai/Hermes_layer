//
// Created by Yifei Yang on 2/4/22.
//

#include <doctest/doctest.h>
#include "TestUtil.h"

namespace fpdb::main::test {

#define SKIP_SUITE false

TEST_SUITE ("tpch-sf0.01-distributed" * doctest::skip(SKIP_SUITE)) {

int parallelDegreeDist = 2;

TEST_CASE ("tpch-sf0.01-distributed-01" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/01.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-02" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/02.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-03" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/03.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-04" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/04.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-05" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/05.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-06" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/06.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-07" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/07.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-08" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/08.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-09" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/09.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-10" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/10.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-11" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/11.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-12" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/12.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-13" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/13.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-14" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/14.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-15" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/15.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-16" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/16.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-17" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/17.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-18" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/18.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-19" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/19.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-20" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/20.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-21" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/21.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf0.01-distributed-22" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf0.01/csv/",
                                                    {"tpch/original/22.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

}

TEST_SUITE ("tpch-sf10-distributed" * doctest::skip(SKIP_SUITE)) {

int parallelDegreeDist = 23;

TEST_CASE ("tpch-sf10-distributed-01" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/01.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-02" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/02.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-03" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/03.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-04" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/04.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-05" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/05.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-06" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/06.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-07" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/07.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-08" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/08.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-09" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/09.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-10" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/10.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-11" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/11.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-12" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/12.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-13" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/13.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-14" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/14.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-15" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/15.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-16" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/16.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-17" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/17.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-18" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/18.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-19" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/19.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-20" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/20.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-21" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/21.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

TEST_CASE ("tpch-sf10-distributed-22" * doctest::skip(false || SKIP_SUITE)) {
          REQUIRE(TestUtil::e2eNoStartCalciteServer("tpch-sf10/csv/",
                                                    {"tpch/original/22.sql"},
                                                    parallelDegreeDist,
                                                    true));
}

}

}
