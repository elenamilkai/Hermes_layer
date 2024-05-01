project(fpdb-hermesserver VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)

#-----------------------------------------------------------------------------------------------------------------------
# Hermes server thrift - cpp
#-----------------------------------------------------------------------------------------------------------------------

add_custom_command(
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/thrift/HermesDuckDBServer.thrift
        OUTPUT
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/HermesDuckDBServer.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/HermesDuckDBServer_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/HermesDuckDBServer_server.skeleton.cpp
        COMMAND thrift_executable --gen cpp -r -o ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/thrift/HermesDuckDBServer.thrift)

add_library(hermesduckdbserver_thrift_cpp STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/HermesDuckDBServer.cpp
        #${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/HermesDuckDBServer_types.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/gen-cpp/HermesDuckDBServer_server.skeleton.cpp)

target_link_libraries(hermesduckdbserver_thrift_cpp PRIVATE thrift_static)
target_link_libraries(hermesduckdbserver_thrift_cpp PRIVATE boost)
#-----------------------------------------------------------------------------------------------------------------------
# Target
#-----------------------------------------------------------------------------------------------------------------------

add_library(fpdb-hermesserver STATIC
        src/HermesServerHandler.cpp include/fpdb/hermesserver/HermesServerHandler.h
        src/Server.cpp include/fpdb/hermesserver/Server.h)

target_include_directories(fpdb-hermesserver PUBLIC include)

target_link_libraries(fpdb-hermesserver PUBLIC hermesduckdbserver_thrift_cpp)
target_link_libraries(fpdb-hermesserver PRIVATE thrift_static)
#target_link_libraries(fpdb-hermesserver PRIVATE thriftnb_static)
target_link_libraries(fpdb-hermesserver PUBLIC boost)
target_link_libraries(fpdb-hermesserver PUBLIC fmt::fmt)
target_link_libraries(fpdb-hermesserver PUBLIC fpdb-executor)


