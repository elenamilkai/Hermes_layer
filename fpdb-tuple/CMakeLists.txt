project(fpdb-tuple VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)


#-------------------------------------------------------------------------------------------------------------
# Target
#-----------------------------------------------------------------------------------------------------------------------

add_library(fpdb-tuple STATIC
        include/fpdb/tuple/Globals.h
        src/TupleSet.cpp include/fpdb/tuple/TupleSet.h
        src/Scalar.cpp include/fpdb/tuple/Scalar.h
        src/Column.cpp include/fpdb/tuple/Column.h
        src/Schema.cpp include/fpdb/tuple/Schema.h
        src/ColumnIterator.cpp include/fpdb/tuple/ColumnIterator.h
        src/ColumnIndex.cpp include/fpdb/tuple/ColumnIndex.h
        src/TupleSetShowOptions.cpp include/fpdb/tuple/TupleSetShowOptions.h
        src/ColumnName.cpp include/fpdb/tuple/ColumnName.h
        src/ColumnBuilder.cpp include/fpdb/tuple/ColumnBuilder.h
        src/Sample.cpp include/fpdb/tuple/Sample.h
        src/arrow/Arrays.cpp include/fpdb/tuple/arrow/Arrays.h
        include/fpdb/tuple/arrow/ArrayHelper.h
        include/fpdb/tuple/arrow/TableHelper.h
        include/fpdb/tuple/arrow/ScalarHelperImpl.h
        include/fpdb/tuple/arrow/ScalarHelperBuilder.h
        include/fpdb/tuple/arrow/ScalarHelper.h
        src/arrow/SchemaHelper.cpp include/fpdb/tuple/arrow/SchemaHelper.h
        src/csv/CSVParser.cpp include/fpdb/tuple/csv/CSVParser.h
        src/ArrayAppender.cpp include/fpdb/tuple/ArrayAppender.h
        src/ArrayHasher.cpp include/fpdb/tuple/ArrayHasher.h
        include/fpdb/tuple/TupleKey.h
        src/TupleSetIndex.cpp include/fpdb/tuple/TupleSetIndex.h
        src/TupleSetIndexFinder.cpp include/fpdb/tuple/TupleSetIndexFinder.h
        src/Converter.cpp include/fpdb/tuple/Converter.h
        src/parquet/ParquetReader.cpp include/fpdb/tuple/parquet/ParquetReader.h
        src/FileReader.cpp include/fpdb/tuple/FileReader.h
        src/FileType.cpp include/fpdb/tuple/FileType.h
        src/FileReaderBuilder.cpp include/fpdb/tuple/FileReaderBuilder.h
        src/csv/CSVReader.cpp include/fpdb/tuple/csv/CSVReader.h
        src/ArrayAppenderWrapper.cpp include/fpdb/tuple/ArrayAppenderWrapper.h
        src/Util.cpp include/fpdb/tuple/Util.h
        src/arrow/ArrowAWSInputStream.cpp include/fpdb/tuple/arrow/ArrowAWSInputStream.h
        src/arrow/ArrowAWSGZIPInputStream.cpp include/fpdb/tuple/arrow/ArrowAWSGZIPInputStream.h
        src/arrow/ArrowAWSGZIPInputStream2.cpp include/fpdb/tuple/arrow/ArrowAWSGZIPInputStream2.h
        src/arrow/CSVToArrowSIMDStreamParser.cpp include/fpdb/tuple/arrow/CSVToArrowSIMDStreamParser.h
        src/arrow/SIMDParserHelpers.cpp include/fpdb/tuple/arrow/SIMDParserHelpers.h
        src/arrow/CSVToArrowSIMDChunkParser.cpp include/fpdb/tuple/arrow/CSVToArrowSIMDChunkParser.h
        src/serialization/ArrowSerializer.cpp include/fpdb/tuple/serialization/ArrowSerializer.h
        include/fpdb/tuple/serialization/TupleKeyElementSerializer.h
        include/fpdb/tuple/serialization/FileReaderSerializer.h)

target_include_directories(fpdb-tuple PUBLIC include)

target_link_libraries(fpdb-tuple PUBLIC fpdb-util)
target_link_libraries(fpdb-tuple PUBLIC fpdb-caf)
target_link_libraries(fpdb-tuple PUBLIC spdlog::spdlog)
target_link_libraries(fpdb-tuple PUBLIC fmt::fmt)
target_link_libraries(fpdb-tuple PUBLIC LLVM-filesystem)
target_link_libraries(fpdb-tuple PUBLIC expected)
target_link_libraries(fpdb-tuple PUBLIC arrow_static)
target_link_libraries(fpdb-tuple PUBLIC parquet_static)
target_link_libraries(fpdb-tuple PUBLIC deflate)
target_link_libraries(fpdb-tuple PUBLIC zlibstatic)


#-----------------------------------------------------------------------------------------------------------------------
# Test
#-----------------------------------------------------------------------------------------------------------------------

add_executable(fpdb-tuple-test
        test/Globals.h
        test/MainTest.cpp
        test/TupleTest.cpp
        test/CSVTest.cpp
        test/ParquetTest.cpp
        test/ArrayAppenderTest.cpp
        test/TupleSetIndexTest.cpp)


target_include_directories(fpdb-tuple-test PRIVATE include)

target_link_libraries(fpdb-tuple-test spdlog::spdlog)
target_link_libraries(fpdb-tuple-test doctest::doctest)
target_link_libraries(fpdb-tuple-test Backward::Backward)
target_link_libraries(fpdb-tuple-test fmt::fmt)
target_link_libraries(fpdb-tuple-test fpdb-tuple)
target_link_libraries(fpdb-tuple-test fpdb-util)


#-----------------------------------------------------------------------------------------------------------------------
# Benchmark
#-----------------------------------------------------------------------------------------------------------------------

add_executable(fpdb-tuple-benchmark
        bench/Globals.h
        bench/Main.cpp
        bench/ArrayAppenderBench.cpp)


target_include_directories(fpdb-tuple-benchmark PRIVATE include)

target_link_libraries(fpdb-tuple-benchmark spdlog::spdlog)
target_link_libraries(fpdb-tuple-benchmark doctest::doctest)
target_link_libraries(fpdb-tuple-benchmark Backward::Backward)
target_link_libraries(fpdb-tuple-benchmark nanobench::nanobench)
target_link_libraries(fpdb-tuple-benchmark fmt::fmt)
target_link_libraries(fpdb-tuple-benchmark fpdb-tuple)


#-----------------------------------------------------------------------------------------------------------------------
# Test Data
#-----------------------------------------------------------------------------------------------------------------------

configure_file(data/csv/test.csv ${CMAKE_CURRENT_BINARY_DIR}/data/csv/test.csv COPYONLY)
configure_file(data/csv/test3x10000.csv ${CMAKE_CURRENT_BINARY_DIR}/data/csv/test3x10000.csv COPYONLY)

#-----------------------------------------------------------------------------------------------------------------------
# Diagnostics
#-----------------------------------------------------------------------------------------------------------------------

#showTargetProps(fpdb-sql)
