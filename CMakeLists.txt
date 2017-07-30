cmake_minimum_required(VERSION 3.0.0)
project(cpp-futures-promises)

set(boost_VERSION "1.64.0")
set(boost_VERSION_UNDERSCORE "1_64_0")
set(folly_VERSION "v2017.07.24.00")
set(wangle_VERSION "v2017.07.24.00")

message(STATUS "Module path: ${CMAKE_MODULE_PATH}")

enable_testing()

include(CTest)

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG(-std=c++1z HAS_CXX_17)
CHECK_CXX_COMPILER_FLAG(-std=c++14 HAS_CXX_14)
CHECK_CXX_COMPILER_FLAG(-std=c++11 HAS_CXX_11)

# Try to use standard C++17 but if it is not available fallback until C++11.
# The environment variables are required for the Bash scripts which often compile external libraries.
if (HAS_CXX_17)
	add_compile_options(-std=c++1z)
	set(ENV{CFLAGS} "-std=c11")
	set(ENV{CXXFLAGS} "-std=c++1z")
elseif (HAS_CXX_14)
	add_compile_options(-std=c++14)
	set(ENV{CFLAGS} "-std=c11")
	set(ENV{CXXFLAGS} "-std=c++14")
elseif (HAS_CXX_11)
	add_compile_options(-std=c++11)
	set(ENV{CFLAGS} "-std=c11")
	set(ENV{CXXFLAGS} "-std=c++11")
endif ()

add_compile_options(-Wall)
# These flags are required for Boost.Thread and Boost.Test:
add_definitions(-DBOOST_THREAD_VERSION=4 -DBOOST_THREAD_PROVIDES_EXECUTORS -DBOOST_TEST_DYN_LINK)
# Workaround for Fedora:
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")

# Bash is required for many of the build scripts for the external projects.
find_program(BASH_PATH bash)

if (NOT BASH_PATH)
	message(FATAL_ERROR "bash not found.")
endif ()

include(ExternalProject)

# Boost
set(BOOST_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/src/boost/")
set(BOOST_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/boost_install")
set(BOOST_INCLUDE_DIR "${BOOST_INSTALL_DIR}/include")
set(BOOST_LIB_DIR "${BOOST_INSTALL_DIR}/lib")

ExternalProject_Add(
	boost
	URL "https://sourceforge.net/projects/boost/files/boost/${boost_VERSION}/boost_${boost_VERSION_UNDERSCORE}.tar.bz2"
	INSTALL_COMMAND "" # Disable install step
	UPDATE_COMMAND "" # Doesn't change often
	CONFIGURE_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/configureboost.sh" "${BOOST_DOWNLOAD_DIR}" "${BOOST_INSTALL_DIR}"
	BUILD_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/buildboost.sh" "${BOOST_DOWNLOAD_DIR}" "${BOOST_LIB_DIR}"
	LOG_DOWNLOAD ON
	LOG_CONFIGURE ON
	LOG_BUILD ON
)

ExternalProject_Add_StepDependencies(boost configure "${CMAKE_CURRENT_SOURCE_DIR}/configureboost.sh")
ExternalProject_Add_StepDependencies(boost build "${CMAKE_CURRENT_SOURCE_DIR}/buildboost.sh")

include_directories("${BOOST_INCLUDE_DIR}")
link_directories("${BOOST_LIB_DIR}")

set(Boost_LIBRARIES "${BOOST_LIB_DIR}/libboost_thread.so" "${BOOST_LIB_DIR}/libboost_system.so" "${BOOST_LIB_DIR}/libboost_chrono.so" "${BOOST_LIB_DIR}/libboost_unit_test_framework.so")

# Folly

# See https://github.com/facebook/folly/blob/master/README.md for all dependencies which have to be installed on Fedora as well.
find_package(gflags REQUIRED)
find_library(GLOG_LIBRARY glog)

if (NOT GLOG_LIBRARY)
	message(FATAL_ERROR "Library glog not found.")
endif ()

set(folly_DIR "${CMAKE_CURRENT_BINARY_DIR}/folly-prefix/src/folly/folly")
set(folly_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/folly_install")
set(folly_INCLUDE_DIR "${folly_INSTALL_DIR}/include")
set(folly_LIB_DIR "${folly_INSTALL_DIR}/lib")

set(folly_LIBRARIES
	"${folly_LIB_DIR}/libfolly.so"
	"${folly_LIB_DIR}/libfollybenchmark.so"
	"${folly_LIB_DIR}/libfollyinit.so"
	"${folly_LIB_DIR}/libfollylogging.so"
	${GLOG_LIBRARY}
)

ExternalProject_Add(folly
	URL "https://github.com/facebook/folly/archive/${folly_VERSION}.tar.gz"
	INSTALL_COMMAND ""
	CONFIGURE_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/configurefolly.sh" "${folly_DIR}" "${folly_INSTALL_DIR}" "${folly_LIB_DIR}/libfolly.so" "${BOOST_INSTALL_DIR}"
	BUILD_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/buildfolly.sh" "${folly_DIR}" "${folly_INSTALL_DIR}" "${folly_LIB_DIR}/libfolly.so"
	DEPENDS boost
)

include_directories("${folly_INCLUDE_DIR}")
link_directories("${folly_LIB_DIR}")

# Wangle is required for the Thread Pool Executors which work with Folly
set(wangle_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/wangle_install")
set(wangle_BUILD_DIR "${CMAKE_BINARY_DIR}/wangle-prefix/src/wangle-build")

ExternalProject_Add(wangle
	URL "https://github.com/facebook/wangle/archive/${wangle_VERSION}.tar.gz"
	INSTALL_COMMAND ""
	CONFIGURE_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/configurewangle.sh" "${folly_LIB_DIR}" "${folly_INCLUDE_DIR}" "${BOOST_INSTALL_DIR}" "${wangle_INSTALL_DIR}"
	BUILD_COMMAND "${BASH_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/buildwangle.sh" "${wangle_BUILD_DIR}"
	DEPENDS folly boost
)

set(wangle_INCLUDE_DIR "${wangle_INSTALL_DIR}/include/")
set(wangle_LIB_DIR "${wangle_INSTALL_DIR}/lib/")
set(wangle_LIBRARIES "${wangle_LIB_DIR}/libwangle.so")

include_directories("${wangle_INCLUDE_DIR}")

# The file compile_commands.json is require by clang-check and has to be exported.
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=1)

add_subdirectory(src)

# memcheck
find_program(VALGRIND_COMMAND valgrind)
if (NOT VALGRIND_COMMAND)
	message(FATAL_ERROR "Did not find valgrind.")
endif ()
set(CTEST_MEMORYCHECK_COMMAND "${VALGRIND_COMMAND} --tool=memcheck")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full") # --xml=yes --fullpath-after=${CMAKE_BINARY_DIR}
set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_SOURCE_DIR}/valgrind_suppress.txt")

set(futures_URL "https://github.com/tdauth/cpp-futures-promises")
set(futures_VERSION_MAJOR "1")
set(futures_VERSION_MINOR "0")
set(futures_VERSION_PATCH "0")
set(futures_VERSION "${futures_VERSION_MAJOR}.${futures_VERSION_MINOR}.${futures_VERSION_PATCH}")

set(CPACK_PACKAGE_NAME "cpp-futures-promises")
set(CPACK_PACKAGE_VENDOR "${futures_URL}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Futures and promises in C++.")
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