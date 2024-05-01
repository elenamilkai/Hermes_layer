project(fpdb-logparser VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)

#-----------------------------------------------------------------------------------------------------------------------
# LogParser server thrift - cpp
#-----------------------------------------------------------------------------------------------------------------------

add_custom_command(
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../thrift/LogParserServer.thrift
        OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/LogParserServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/LogParserServer_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/LogParserServer_server.skeleton.cpp
        COMMAND thrift_executable --gen cpp -r -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../thrift/LogParserServer.thrift)

add_library(logparserserver_thrift_cpp STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/LogParserServer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/LogParserServer_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/LogParserServer_server.skeleton.cpp)

target_link_libraries(logparserserver_thrift_cpp PRIVATE boost)
target_link_libraries(logparserserver_thrift_cpp PRIVATE thrift_static)

#-----------------------------------------------------------------------------------------------------------------------
# LogParser server thrift - java
#-----------------------------------------------------------------------------------------------------------------------
add_custom_target(
        logparserserver_thrift_java ALL
        COMMAND thrift_executable --gen java -r -out ${CMAKE_CURRENT_SOURCE_DIR}/../java/main/java ${CMAKE_CURRENT_SOURCE_DIR}/../thrift/LogParserServer.thrift
        COMMAND mvn clean install -f ${CMAKE_SOURCE_DIR}/fpdb-logparser/java/pom.xml -DskipTests)

#-----------------------------------------------------------------------------------------------------------------------
# Target
#-----------------------------------------------------------------------------------------------------------------------

add_library(fpdb-logparser STATIC
        src/LogParserClient.cpp include/fpdb/logparser/LogParserClient.h
        ../java/main/resources/config/exec.conf
        src/deltapump/makeTuple.cpp include/deltapump/makeTuple.h
        src/deltapump/BinlogParser.cpp include/deltapump/BinlogParser.h
        include/deltapump/customer_d.hh
        include/deltapump/date_d.hh
        include/deltapump/lineorder_d.hh
        include/deltapump/part_d.hh
        include/deltapump/supplier_d.hh
        include/deltapump/history_d.hh
        test/parserTest.cpp)

target_include_directories(fpdb-logparser PUBLIC include)

target_link_libraries(fpdb-logparser PUBLIC logparserserver_thrift_cpp)
target_link_libraries(fpdb-logparser PUBLIC thrift_static)
target_link_libraries(fpdb-logparser PUBLIC boost)
target_link_libraries(fpdb-logparser PUBLIC fmt::fmt)

add_dependencies(fpdb-logparser logparserserver_thrift_java)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include/deltapump/lib
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/include/deltapump/schemas
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------------------------------------------------
# Test
#-----------------------------------------------------------------------------------------------------------------------
add_executable(fpdb-logparser-test
        test/parserTest.cpp
        # src
        src/LogParserClient.cpp
        src/deltapump/BinlogParser.cpp
        src/deltapump/makeTuple.cpp
        # header
        include/fpdb/logparser/LogParserClient.h
        include/deltapump/BinlogParser.h
        include/deltapump/makeTuple.h
        # schema
        include/deltapump/lineorder_d.hh
        include/deltapump/customer_d.hh
        include/deltapump/supplier_d.hh
        include/deltapump/part_d.hh
        include/deltapump/date_d.hh
        include/deltapump/history_d.hh)

target_link_libraries(fpdb-logparser-test PUBLIC fpdb-logparser)
target_link_libraries(fpdb-logparser-test PUBLIC /usr/local/lib/libavrocpp.so)
