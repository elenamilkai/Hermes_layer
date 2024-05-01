project(fpdb-main VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)


#-----------------------------------------------------------------------------------------------------------------------
# Targets
#-----------------------------------------------------------------------------------------------------------------------

# fpdb-main-client
add_library(fpdb-main
        src/ExecConfig.cpp include/fpdb/main/ExecConfig.h
        include/fpdb/main/ActorSystemConfig.h
        src/CAFInit.cpp include/fpdb/main/CAFInit.h
        src/Client.cpp include/fpdb/main/Client.h
        src/Server.cpp include/fpdb/main/Server.h)

target_include_directories(fpdb-main PUBLIC include)

target_link_libraries(fpdb-main PUBLIC fpdb-hermesserver)
target_link_libraries(fpdb-main PUBLIC fpdb-executor)
target_link_libraries(fpdb-main PUBLIC fpdb-util)
target_link_libraries(fpdb-main PUBLIC ssl crypto)
target_link_libraries(fpdb-main PUBLIC fpdb-logparser)


# fpdb-main-client
add_executable(fpdb-main-client
        src/ClientMain.cpp)

target_link_libraries(fpdb-main-client PRIVATE fpdb-main)
# need to add and link libraries on mac
if (${APPLE})
  linkForMacExecutable(fpdb-main-client)
endif()


# fpdb-main-server
add_executable(fpdb-main-server
        src/ServerMain.cpp)

target_link_libraries(fpdb-main-server PRIVATE fpdb-main)
if (${APPLE})
  linkForMacExecutable(fpdb-main-server)
endif()


#-----------------------------------------------------------------------------------------------------------------------
# Test
#-----------------------------------------------------------------------------------------------------------------------

add_executable(fpdb-main-test
        test/Globals.h
        test/TestUtil.cpp test/TestUtil.h
        test/MainTest.cpp
        test/HTAP-SF1-Test.cpp
        test/HTAP-SF10-Test.cpp
        test/TPCHTest.cpp
        test/TPCHDistTest.cpp
        test/HTAPValTest.cpp
        test/OnePartitionSSB.cpp test/HTAP-SF10-Test.cpp test/HTAP-SF50-Test.cpp test/HTAP-SF50-SmallPartitions-Test.cpp)

target_link_libraries(fpdb-main-test PRIVATE fpdb-main)
target_link_libraries(fpdb-main-test PRIVATE doctest::doctest)
# need to add and link libraries on mac
if (${APPLE})
  linkForMacExecutable(fpdb-main-test)
endif()
