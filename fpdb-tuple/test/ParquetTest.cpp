//
// Created by matt on 11/8/20.
//


#include <memory>
#include <experimental/filesystem>

#include <doctest/doctest.h>
#include <fmt/format.h>

#include <fpdb/tuple/Converter.h>
#include <fpdb/tuple/Globals.h>
#include <fpdb/tuple/parquet/ParquetReader.h>
#include <fpdb/util/Util.h>

using namespace fpdb::tuple;
using namespace fpdb::util;

const char *getCurrentTestName();
const char *getCurrentTestSuiteName();

#define SKIP_SUITE false

tl::expected<void, std::string> convert(const std::string& inFile, const std::string& outFile,  int rowGroupSize) {

  std::experimental::filesystem::create_directories(std::experimental::filesystem::absolute(outFile).remove_filename());

  auto fields = {::arrow::field("A", ::arrow::int32()),
				 ::arrow::field("B", ::arrow::int32()),
				 ::arrow::field("C", ::arrow::int32())};

  auto schema = std::make_shared<::arrow::Schema>(fields);

  auto result = Converter::csvToParquet(inFile, outFile, *schema, rowGroupSize, ::parquet::Compression::type::SNAPPY);

  return result;
}

TEST_SUITE ("parquet" * doctest::skip(SKIP_SUITE)) {

TEST_CASE ("parquet-csv-to-parquet" * doctest::skip(false || SKIP_SUITE)) {
  // supplier
  /*const std::string inFile_supplier = "/home/ubuntu/data/supplier.tbl";
  cout << inFile_supplier << endl;
  const std::string outFile_supplier = fmt::format("/home/ubuntu/parquet/supplier.parquet", getCurrentTestSuiteName(), getCurrentTestName());
  cout << outFile_supplier << endl;

  auto fields_supplier = {::arrow::field("s_suppkey", ::arrow::int64()),
                          ::arrow::field("s_name", ::arrow::utf8()),
                          ::arrow::field("s_address", ::arrow::utf8()),
                          ::arrow::field("s_city", ::arrow::utf8()),
                          ::arrow::field("s_nation", ::arrow::utf8()),
                          ::arrow::field("s_region", ::arrow::utf8()),
                          ::arrow::field("s_phone", ::arrow::utf8()),
                          ::arrow::field("s_ytd", ::arrow::int64())};

  auto schema = std::make_shared<::arrow::Schema>(fields_supplier);

  auto result = Converter::csvToParquet(inFile_supplier, outFile_supplier, *schema, DefaultChunkSize, ::parquet::Compression::type::SNAPPY);
  //auto result = convert(inFile_supplier, outFile_supplier, DefaultChunkSize);
  	  CHECK_MESSAGE(result.has_value(), result.error());*/

  // lineorder
  for(int j=49; j<=49; j++) {
    std::string inFile_lo = "/home/ubuntu/data/lineorder.tbl."+ to_string(j);
    cout << inFile_lo << endl;
    std::string outFile_lo = fmt::format("/home/ubuntu/parquet/lineorder.parquet."+ to_string(j));
    cout << outFile_lo << endl;

    auto fields_lo = {::arrow::field("lo_orderkey", ::arrow::int64()),
                      ::arrow::field("lo_linenumber", ::arrow::int64()),
                      ::arrow::field("lo_custkey", ::arrow::int64()),
                      ::arrow::field("lo_partkey", ::arrow::int64()),
                      ::arrow::field("lo_suppkey", ::arrow::int64()),
                      ::arrow::field("lo_orderdate", ::arrow::int64()),
                      ::arrow::field("lo_orderpriority", ::arrow::utf8()),
                      ::arrow::field("lo_shippriority", ::arrow::utf8()),
                      ::arrow::field("lo_quantity", ::arrow::int64()),
                      ::arrow::field("lo_extendedprice", ::arrow::int64()),
                      ::arrow::field("lo_discount", ::arrow::int64()),
                      ::arrow::field("lo_revenue", ::arrow::int64()),
                      ::arrow::field("lo_supplycost", ::arrow::int64()),
                      ::arrow::field("lo_tax", ::arrow::int64()),
                      ::arrow::field("lo_commitdate", ::arrow::int64()),
                      ::arrow::field("lo_shipmode", ::arrow::utf8())};

    auto schema = std::make_shared<::arrow::Schema>(fields_lo);

    auto result = Converter::csvToParquet(inFile_lo, outFile_lo, *schema, DefaultChunkSize, ::parquet::Compression::type::SNAPPY);
    //auto result = convert(inFile_supplier, outFile_supplier, DefaultChunkSize);
    CHECK_MESSAGE(result.has_value(), result.error());
  }

  // part
  /*const std::string inFile_p = "/home/ubuntu/data/part.tbl";
  cout << inFile_p << endl;
  const std::string outFile_p = fmt::format("/home/ubuntu/parquet/part.parquet");
  cout << outFile_p << endl;

  auto fields_p = {::arrow::field("p_partkey", ::arrow::int64()),
                          ::arrow::field("p_name", ::arrow::utf8()),
                          ::arrow::field("p_mfgr", ::arrow::utf8()),
                          ::arrow::field("p_category", ::arrow::utf8()),
                          ::arrow::field("p_brand1", ::arrow::utf8()),
                          ::arrow::field("p_color", ::arrow::utf8()),
                          ::arrow::field("p_type", ::arrow::utf8()),
                          ::arrow::field("p_size", ::arrow::int64()),
                          ::arrow::field("p_container", ::arrow::utf8()),
                          ::arrow::field("p_price", ::arrow::int64())};

  auto schema_p = std::make_shared<::arrow::Schema>(fields_p);

  auto result = Converter::csvToParquet(inFile_p, outFile_p, *schema_p, DefaultChunkSize, ::parquet::Compression::type::SNAPPY);
  //auto result = convert(inFile_supplier, outFile_supplier, DefaultChunkSize);
  CHECK_MESSAGE(result.has_value(), result.error());
  */
  // customer
  /*const std::string inFile_c = "/home/ubuntu/data/customer.tbl";
  cout << inFile_c << endl;
  const std::string outFile_c = fmt::format("/home/ubuntu/parquet/customer.parquet", getCurrentTestSuiteName(), getCurrentTestName());
  cout << outFile_c << endl;

  auto fields_c = {::arrow::field("c_custkey", ::arrow::int64()),
                   ::arrow::field("c_name", ::arrow::utf8()),
                   ::arrow::field("c_address", ::arrow::utf8()),
                   ::arrow::field("c_city", ::arrow::utf8()),
                   ::arrow::field("c_nation", ::arrow::utf8()),
                   ::arrow::field("c_region", ::arrow::utf8()),
                   ::arrow::field("c_phone", ::arrow::utf8()),
                   ::arrow::field("c_mktsegment", ::arrow::utf8()),
                   ::arrow::field("c_paymentcnt", ::arrow::int64())};

  auto schema_c = std::make_shared<::arrow::Schema>(fields_c);

  auto result = Converter::csvToParquet(inFile_c, outFile_c, *schema_c, DefaultChunkSize, ::parquet::Compression::type::SNAPPY);
  //auto result = convert(inFile_supplier, outFile_supplier, DefaultChunkSize);
  CHECK_MESSAGE(result.has_value(), result.error());
  */
  // date
  /*const std::string inFile_d = "/home/ubuntu/data/date.tbl";
  cout << inFile_d << endl;
  const std::string outFile_d = fmt::format("tests/{}/{}/date.parquet", getCurrentTestSuiteName(), getCurrentTestName());
  cout << outFile_d << endl;

  auto fields_d = {::arrow::field("d_datekey", ::arrow::int64()),
                   ::arrow::field("d_date", ::arrow::utf8()),
                   ::arrow::field("d_dayofweek", ::arrow::utf8()),
                   ::arrow::field("d_month", ::arrow::utf8()),
                   ::arrow::field("d_year", ::arrow::int64()),
                   ::arrow::field("d_yearmonthnum", ::arrow::int64()),
                   ::arrow::field("d_yearmonth", ::arrow::utf8()),
                   ::arrow::field("d_daynuminweek", ::arrow::int64()),
                   ::arrow::field("d_daynuminmonth", ::arrow::int64()),
                   ::arrow::field("d_daynuminyear", ::arrow::int64()),
                   ::arrow::field("d_monthnuminyear", ::arrow::int64()),
                   ::arrow::field("d_weeknuminyear", ::arrow::int64()),
                   ::arrow::field("d_sellingseason", ::arrow::utf8()),
                   ::arrow::field("d_lastdayinweekfl", ::arrow::boolean()),
                   ::arrow::field("d_lastdayinmonthfl", ::arrow::boolean()),
                   ::arrow::field("d_holidayfl", ::arrow::boolean()),
                   ::arrow::field("d_weekdayfl", ::arrow::boolean())};

  auto schema_d = std::make_shared<::arrow::Schema>(fields_d);

  auto result = Converter::csvToParquet(inFile_d, outFile_d, *schema_d, DefaultChunkSize, ::parquet::Compression::type::SNAPPY);
  //auto result = convert(inFile_supplier, outFile_supplier, DefaultChunkSize);
  CHECK_MESSAGE(result.has_value(), result.error());
  */
}

/*TEST_CASE ("parquet-read-byte-range" * doctest::skip(false || SKIP_SUITE)) {

  const std::string inFile = "data/csv/test3x10000.csv";
  const std::string outFile = fmt::format("tests/{}/{}/test3x10000.snappy.parquet", getCurrentTestSuiteName(), getCurrentTestName());

  auto result = convert(inFile, outFile, 300);
	  CHECK_MESSAGE(result.has_value(), result.error());

  auto size = std::experimental::filesystem::file_size(outFile);
  auto scanRanges = fpdb::util::ranges<int>(0, size, 3);

  auto expectedReader = ParquetReader::make(outFile);
  if(!expectedReader)
		FAIL (expectedReader.error());
  auto reader = expectedReader.value();

  for (const auto &scanRange: scanRanges) {
	auto expectedTupleSet = reader->read({"A","B","C"}, scanRange.first, scanRange.second);
	if (!expectedTupleSet)
		  FAIL (expectedTupleSet.error());
	auto tupleSet = expectedTupleSet.value();
	SPDLOG_DEBUG("Output:\n{}", tupleSet->showString(TupleSetShowOptions(TupleSetShowOrientation::RowOriented)));
  }
}*/

}
