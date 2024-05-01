cmake_minimum_required(VERSION 3.15)
project("fpdb" VERSION "${PROJECT_VERSION}" LANGUAGES C CXX)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_LIST_DIR}/cmake")

include(CMakeUtils)

setDefaults()

include(Configure_LLVM)


#-----------------------------------------------------------------------------------------------------------------------
# Set openssl path for mac os
#-----------------------------------------------------------------------------------------------------------------------

if (${APPLE})
  set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
  # This is a bug in CMake that causes it to prefer the system version over
  # the one in the specified ROOT folder.
  set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib CACHE FILEPATH "" FORCE)
  set(OPENSSL_SSL_LIBRARY ${OPENSSL_ROOT_DIR}/lib/libssl.dylib CACHE FILEPATH "" FORCE)
  SET(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
  SET(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
endif()


#-----------------------------------------------------------------------------------------------------------------------
# Function for additional commands for mac os
#-----------------------------------------------------------------------------------------------------------------------

function(linkForMacExecutable target)
  # missing linking on mac
  target_link_directories(${target} PRIVATE /usr/local/opt/openssl/lib)
  target_link_directories(${target} PRIVATE /usr/local/opt/llvm/lib)
  target_link_directories(${target} PRIVATE /usr/local/opt/libdeflate/lib)

  # remove -lrt from link.txt which mac does not have or need
  set(linkFile ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target}.dir/link.txt)
  add_custom_command(TARGET ${target} PRE_LINK
          COMMAND sed -i '' 's/ -lrt//g' ${linkFile})

  # remove /libc++fs.a from build.make and link.txt which mac does not have or need
  set(buildFile ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${target}.dir/build.make)
  add_custom_target(pre-compile-${target}
          COMMAND sed -i '' '/\\/libc++fs.a/d' ${buildFile}
          COMMAND sed -i '' 's/ \\/libc++fs.a//g' ${linkFile})
  add_dependencies(${target} pre-compile-${target})

  # fix "dyld: Library not loaded" on mac
  set(path_to_libs libgvplugin_neato_layout.6.dylib
          libgvplugin_core.6.dylib
          @rpath/libaws-cpp-sdk-core.dylib
          @rpath/libaws-cpp-sdk-s3.dylib
          @rpath/libaws-crt-cpp.dylib
          @rpath/libcaf_core.0.18.5.dylib
          @rpath/libcaf_io.0.18.5.dylib)
  set(change_libs ${CMAKE_BINARY_DIR}/_deps/graphviz_ep/install/lib/graphviz/libgvplugin_neato_layout.6.dylib
          ${CMAKE_BINARY_DIR}/_deps/graphviz_ep/install/lib/graphviz/libgvplugin_core.6.dylib
          ${CMAKE_BINARY_DIR}/_deps/aws-cpp-sdk_ep/install/lib/libaws-cpp-sdk-core.dylib
          ${CMAKE_BINARY_DIR}/_deps/aws-cpp-sdk_ep/install/lib/libaws-cpp-sdk-s3.dylib
          ${CMAKE_BINARY_DIR}/_deps/aws-cpp-sdk_ep/install/lib/libaws-crt-cpp.dylib
          ${CMAKE_BINARY_DIR}/_deps/caf_ep/install/lib/libcaf_core.0.18.5.dylib
          ${CMAKE_BINARY_DIR}/_deps/caf_ep/install/lib/libcaf_io.0.18.5.dylib)
  foreach(path_to_lib change_lib IN ZIP_LISTS path_to_libs change_libs)
    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND install_name_tool -change ${path_to_lib} ${change_lib} ${target})
  endforeach()
endfunction()


#-----------------------------------------------------------------------------------------------------------------------
# Dependencies
#-----------------------------------------------------------------------------------------------------------------------

set(DEPS_PREFIX "_deps")

include(External_spdlog)
include(External_doctest)
include(External_nanobench)
include(External_caf)
include(External_arrow)
include(External_AWSCPPSDK)
include(External_ANTLR)
include(External_graphviz)
include(External_expected)
include(External_fmt)
include(External_ssb_dbgen)
include(External_primesieve)
include(Dependency_SQLite3)
include(External_Backward)
include(External_CallableTraits)
include(External_zlib_ng)
include(External_boost)
include(External_thrift)
include(External_nlohmannjson)



#-----------------------------------------------------------------------------------------------------------------------
# Modules
#-----------------------------------------------------------------------------------------------------------------------

add_subdirectory(fpdb-util)
add_subdirectory(fpdb-caf)
add_subdirectory(fpdb-tuple)
add_subdirectory(fpdb-aws)
add_subdirectory(fpdb-catalogue)
add_subdirectory(fpdb-expression)
add_subdirectory(fpdb-expression-gandiva)
add_subdirectory(fpdb-calcite/cpp)
add_subdirectory(fpdb-plan)
add_subdirectory(fpdb-cache)
add_subdirectory(fpdb-executor)
add_subdirectory(fpdb-main)
add_subdirectory(fpdb-delta) # Delta Manager for HTAP
add_subdirectory(fpdb-logparser/cpp)
add_subdirectory(fpdb-hermesserver)

#-----------------------------------------------------------------------------------------------------------------------
# Resources
#-----------------------------------------------------------------------------------------------------------------------

file(COPY resources/config/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources/config/)
file(COPY resources/metadata/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources/metadata/)
file(COPY resources/query/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources/query/)
