project(fpdb-util VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)


#-----------------------------------------------------------------------------------------------------------------------
# Util
#-----------------------------------------------------------------------------------------------------------------------

add_library(fpdb-util
        include/fpdb/util/Traits.h
        src/Util.cpp include/fpdb/util/Util.h)

target_include_directories(fpdb-util PUBLIC include)

target_link_libraries(fpdb-util PUBLIC LLVM-filesystem)
target_link_libraries(fpdb-util PUBLIC fmt::fmt)
target_link_libraries(fpdb-util PUBLIC spdlog::spdlog)
target_link_libraries(fpdb-util PUBLIC expected)
target_link_libraries(fpdb-util PUBLIC boost_callable_traits)


#-----------------------------------------------------------------------------------------------------------------------
# Test
#-----------------------------------------------------------------------------------------------------------------------

add_executable(fpdb-util-test
        test/Globals.h
        test/MainTest.cpp
        test/TraitsTest.cpp
        test/UtilTest.cpp)

target_link_libraries(fpdb-util-test fpdb-util)
target_link_libraries(fpdb-util-test fmt::fmt)
target_link_libraries(fpdb-util-test spdlog::spdlog)
target_link_libraries(fpdb-util-test doctest::doctest)
# need to add and link libraries on mac
if (${APPLE})
  linkForMacExecutable(fpdb-util-test)
endif()


#-----------------------------------------------------------------------------------------------------------------------
# Diagnostics
#-----------------------------------------------------------------------------------------------------------------------

#showTargetProps(fpdb-util)
