# Thrift

include(GNUInstallDirs)

set(THRIFT_VERSION "0.17.0")
set(THRIFT_GIT_URL "https://github.com/apache/thrift.git")

include(ExternalProject)
find_package(Git REQUIRED)

set(THRIFT_BASE thrift_ep)
set(THRIFT_PREFIX ${DEPS_PREFIX}/${THRIFT_BASE})
set(THRIFT_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${THRIFT_PREFIX})
set(THRIFT_INSTALL_DIR ${THRIFT_BASE_DIR}/install)
set(THRIFT_INCLUDE_DIR ${THRIFT_INSTALL_DIR}/include)
set(THRIFT_LIB_DIR ${THRIFT_INSTALL_DIR}/lib)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(THRIFT_STATIC_LIB ${THRIFT_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}thriftd${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
  set(THRIFT_STATIC_LIB ${THRIFT_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}thrift${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(THRIFTNB_STATIC_LIB ${THRIFT_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}thriftnbd${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
  set(THRIFTNB_STATIC_LIB ${THRIFT_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}thriftnb${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()
set(_THRIFT_BYPRODUCTS "")
list(APPEND _THRIFT_BYPRODUCTS ${THRIFT_STATIC_LIB})
list(APPEND _THRIFT_BYPRODUCTS ${THRIFT_INSTALL_DIR}${CMAKE_INSTALL_BINDIR}/thrift)

list(APPEND CMAKE_SYSTEM_PREFIX_PATH /usr/local/opt)

find_package(OpenSSL REQUIRED)

find_program(BISON_COMMAND bison PATHS /usr/bin /usr/local/opt)
get_filename_component(BISON_ROOT ${BISON_COMMAND} DIRECTORY)

ExternalProject_Add(${THRIFT_BASE}
        PREFIX ${THRIFT_BASE_DIR}
        GIT_REPOSITORY ${THRIFT_GIT_URL}
        GIT_TAG ${THRIFT_VERSION}
        GIT_PROGRESS ON
        GIT_SHALLOW ON
        UPDATE_DISCONNECTED TRUE
        INSTALL_DIR ${THRIFT_INSTALL_DIR}
        BUILD_BYPRODUCTS ${_THRIFT_BYPRODUCTS}
        CMAKE_ARGS
        -DOPENSSL_INCLUDE_DIR=${OPENSSL_INCLUDE_DIR}
        -DOPENSSL_CRYPTO_LIBRARY=${OPENSSL_CRYPTO_LIBRARY}
        -DOPENSSL_SSL_LIBRARY=${OPENSSL_SSL_LIBRARY}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_MESSAGE=NEVER
        -DCMAKE_INSTALL_PREFIX=${THRIFT_INSTALL_DIR}
        -DBUILD_TESTING=FALSE
        -DBUILD_PYTHON=FALSE
        -DBUILD_NODEJS=FALSE
        -DBUILD_JAVA=FALSE
        -DBUILD_JAVASCRIPT=FALSE
        -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE
        -DBoost_ROOT=${BOOST_INSTALL_DIR}
        -DBoost_INCLUDE_DIR=${BOOST_INCLUDE_DIR}
        -DCMAKE_POLICY_DEFAULT_CMP0074=NEW
        -DBISON_ROOT=${BISON_ROOT}
        )
add_dependencies(${THRIFT_BASE} ${BOOST_BASE})

file(MAKE_DIRECTORY ${THRIFT_INCLUDE_DIR}) # Include directory needs to exist to run configure step

#add_library(thrift_static_ep STATIC IMPORTED)
#set_target_properties(thrift_static_ep PROPERTIES IMPORTED_LOCATION ${THRIFT_STATIC_LIB})
#target_include_directories(thrift_static_ep INTERFACE ${THRIFT_INCLUDE_DIR})
#add_dependencies(thrift_static_ep ${THRIFT_BASE})

#find_package(Threads REQUIRED)
#add_library(thriftnb_static STATIC IMPORTED)
#set_target_properties(thriftnb_static PROPERTIES IMPORTED_LOCATION ${THRIFTNB_STATIC_LIB})
#target_include_directories(thriftnb_static INTERFACE ${THRIFT_INCLUDE_DIR})
#target_link_libraries(thriftnb_static INTERFACE thrift_static)
#target_link_libraries(thriftnb_static INTERFACE /usr/lib/x86_64-linux-gnu/libevent.a)
#target_link_libraries(thriftnb_static INTERFACE Threads::Threads)
#add_dependencies(thriftnb_static ${THRIFT_BASE})

add_executable(thrift_executable IMPORTED)
set_target_properties(thrift_executable PROPERTIES IMPORTED_LOCATION ${THRIFT_INSTALL_DIR}/${CMAKE_INSTALL_BINDIR}/thrift)
add_dependencies(thrift_executable ${THRIFT_BASE})

get_target_property(THRIFT_EXECUTABLE thrift_executable LOCATION)