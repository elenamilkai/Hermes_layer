# Boost

set(BOOST_VERSION "boost-1.77.0")
set(BOOST_GIT_URL "https://github.com/boostorg/boost.git")


include(ExternalProject)
find_package(Git REQUIRED)


set(BOOST_BASE boost_ep)
set(BOOST_PREFIX ${DEPS_PREFIX}/${BOOST_BASE})
set(BOOST_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR}/${BOOST_PREFIX})
set(BOOST_INSTALL_DIR ${BOOST_BASE_DIR}/install)
set(BOOST_INCLUDE_DIR ${BOOST_INSTALL_DIR}/include)

set(_BOOST_INCLUDE_LIBRARIES "")
list(APPEND _BOOST_INCLUDE_LIBRARIES "tokenizer")
list(APPEND _BOOST_INCLUDE_LIBRARIES "numeric/conversion")
list(APPEND _BOOST_INCLUDE_LIBRARIES "algorithm")
list(APPEND _BOOST_INCLUDE_LIBRARIES "locale")

ExternalProject_Add(${BOOST_BASE}
        PREFIX ${BOOST_BASE_DIR}
        GIT_REPOSITORY ${BOOST_GIT_URL}
        GIT_TAG ${BOOST_VERSION}
        GIT_PROGRESS ON
        GIT_SHALLOW ON
        UPDATE_DISCONNECTED TRUE
        INSTALL_DIR ${BOOST_INSTALL_DIR}
        CMAKE_CACHE_ARGS
        -DBOOST_INCLUDE_LIBRARIES:STRING=${_BOOST_INCLUDE_LIBRARIES} # Hack to pass a list to cmake EP
        CMAKE_ARGS
        -DCMAKE_INSTALL_MESSAGE=NEVER
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${BOOST_INSTALL_DIR}
        )

file(MAKE_DIRECTORY ${BOOST_INCLUDE_DIR}) # Include directory needs to exist to run configure step

add_library(boost INTERFACE IMPORTED)
target_include_directories(boost INTERFACE ${BOOST_INCLUDE_DIR})
add_dependencies(boost ${BOOST_BASE})
