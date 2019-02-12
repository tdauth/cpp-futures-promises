cmake_minimum_required(VERSION 3.0.0)
project(cpp-futures-promises)

set(googletest_VERSION "1.8.0")
set(folly_VERSION "v2019.01.28.00")

enable_testing()

include(CTest)

add_compile_options(-std=c++17 -Wall)
# These flags are required for Boost.Thread and Boost.Test:
add_definitions(-DBOOST_THREAD_VERSION=4 -DBOOST_THREAD_PROVIDES_EXECUTORS -DBOOST_TEST_DYN_LINK)

find_package(Boost REQUIRED COMPONENTS thread chrono context program_options regex filesystem date_time) # test

include(ExternalProject)

# Folly

# See https://github.com/facebook/folly/blob/master/README.md for all dependencies which have to be installed on Fedora as well.
# We have to find several dependencies manually and link them with the static Folly library.

find_library(GLOG_LIBRARY glog)

if (NOT GLOG_LIBRARY)
	message(FATAL_ERROR "Library glog not found.")
endif ()

find_library(GFLAGS_LIBRARY gflags)

if (NOT GFLAGS_LIBRARY)
	message(FATAL_ERROR "Library gflags not found.")
endif ()

find_library(DL_LIBRARY dl)

if (NOT DL_LIBRARY)
	message(FATAL_ERROR "Library dl not found.")
endif ()

find_library(EVENT_LIBRARY event)

if (NOT EVENT_LIBRARY)
	message(FATAL_ERROR "Library event not found.")
endif ()

find_library(EVENT_CORE_LIBRARY event_core)

if (NOT EVENT_CORE_LIBRARY)
	message(FATAL_ERROR "Library event_core not found.")
endif ()

find_library(DOUBLE_CONVERSION_LIBRARY double-conversion)

if (NOT DOUBLE_CONVERSION_LIBRARY)
	message(FATAL_ERROR "Library double-conversion not found.")
endif ()

find_library(PTHREAD_LIBRARY pthread)

if (NOT PTHREAD_LIBRARY)
	message(FATAL_ERROR "Library pthread not found.")
endif ()

set(googletest_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/googletest_install")
set(googletest_INCLUDE_DIR "${googletest_INSTALL_DIR}/include")
set(googletest_LIB_DIR "${googletest_INSTALL_DIR}/lib")

set(folly_DIR "${CMAKE_CURRENT_BINARY_DIR}/folly-prefix/src/folly")
set(folly_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/folly_install")
set(folly_INCLUDE_DIR "${folly_INSTALL_DIR}/include")
set(folly_LIB_DIR "${folly_INSTALL_DIR}/lib")

set(folly_LIBRARIES
	"${folly_LIB_DIR}/libfolly.a"
	"${folly_LIB_DIR}/libfollybenchmark.a"
	${GLOG_LIBRARY}
	${GFLAGS_LIBRARY}
	${DL_LIBRARY}
	${EVENT_LIBRARY}
	${EVENT_CORE_LIBRARY}
	${DOUBLE_CONVERSION_LIBRARY}
	${PTHREAD_LIBRARY}
)

ExternalProject_Add(googletest
	URL "https://github.com/google/googletest/archive/release-${googletest_VERSION}.tar.gz"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${googletest_INSTALL_DIR}
	LOG_DOWNLOAD ON
	LOG_CONFIGURE ON
	LOG_BUILD ON
)

ExternalProject_Add(folly
	URL "https://github.com/facebook/folly/archive/${folly_VERSION}.tar.gz"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${folly_INSTALL_DIR} -DCMAKE_INCLUDE_PATH=${googletest_INCLUDE_DIR} -DCMAKE_LIBRARY_PATH=${googletest_LIB_DIR}
	LOG_DOWNLOAD ON
	LOG_CONFIGURE ON
	LOG_BUILD ON
	DEPENDS googletest
)

include_directories("${folly_INCLUDE_DIR}")
link_directories("${folly_LIB_DIR}")

# The file compile_commands.json is require by clang-check and has to be exported.
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=1)

add_subdirectory(src)

# memcheck
find_program(VALGRIND_COMMAND valgrind)
if (NOT VALGRIND_COMMAND)
	message(FATAL_ERROR "Did not find valgrind.")
endif ()
set(CTEST_MEMORYCHECK_COMMAND "${VALGRIND_COMMAND} --tool=memcheck")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/valgrind_suppress.txt")

set(futures_URL "https://github.com/tdauth/cpp-futures-promises")
set(futures_VERSION_MAJOR "1")
set(futures_VERSION_MINOR "0")
set(futures_VERSION_PATCH "0")
set(futures_VERSION "${futures_VERSION_MAJOR}.${futures_VERSION_MINOR}.${futures_VERSION_PATCH}")

set(CPACK_PACKAGE_NAME "cpp-futures-promises")
set(CPACK_PACKAGE_VENDOR "${futures_URL}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced futures and promises in C++.")
set(CPACK_PACKAGE_VERSION "${futures_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${futures_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${futures_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${futures_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "futures")
set(CPACK_PACKAGE_CONTACT "tamino@cdauth.eu")

# Windows NSIS
set(CPACK_NSIS_URL_INFO_ABOUT "${futures_URL}")
# The path page is confusing for normal users and it is not necessary for the game.
set(CPACK_NSIS_MODIFY_PATH OFF)

# Fedora RPM
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
set(CPACK_RPM_PACKAGE_URL "${futures_URL}")

# Debian and Ubuntu DEB
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tamino Dauth")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${futures_URL}")

include(CPack)
