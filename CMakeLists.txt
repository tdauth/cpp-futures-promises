cmake_minimum_required(VERSION 3.8.0)
project(cpp-futures-promises)

set(boost_VERSION "1.64.0")
set(boost_VERSION_UNDERSCORE "1_64_0")
set(folly_VERSION "v2017.07.03.00")
set(wangle_VERSION "v2017.07.03.00")

enable_testing()

include(CTest)

# The thesis code is based on the standard C++17:
add_compile_options(-std=c++1z -Wall)
set(CMAKE_C_COMPILER "/usr/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
# These flags are required for Boost.Thread and Boost.Test:
add_definitions(-DBOOST_THREAD_VERSION=4 -DBOOST_THREAD_PROVIDES_EXECUTORS -DBOOST_TEST_DYN_LINK)
# Workaround for Fedora:
set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")

include(ExternalProject)

# Boost
set(BOOST_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/boost-prefix/src/boost/")
set(BOOST_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/boostinstall")
set(BOOST_INCLUDE_DIR "${BOOST_INSTALL_DIR}/include")
set(BOOST_LIB_DIR "${BOOST_INSTALL_DIR}/lib")

ExternalProject_Add(
	boost
	URL "https://sourceforge.net/projects/boost/files/boost/${boost_VERSION}/boost_${boost_VERSION_UNDERSCORE}.tar.bz2"
	INSTALL_COMMAND "" # Disable install step
	UPDATE_COMMAND "" # Doesn't change often
	CONFIGURE_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/configureboost.sh" "${BOOST_DOWNLOAD_DIR}" "${BOOST_INSTALL_DIR}"
	BUILD_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/buildboost.sh" "${BOOST_DOWNLOAD_DIR}" "${BOOST_LIB_DIR}"
	LOG_DOWNLOAD ON
	LOG_CONFIGURE ON
	LOG_BUILD ON
)

ExternalProject_Add_StepDependencies(boost configure "${CMAKE_CURRENT_SOURCE_DIR}/configureboost.sh")
ExternalProject_Add_StepDependencies(boost build "${CMAKE_CURRENT_SOURCE_DIR}/buildboost.sh")

include_directories("${BOOST_INCLUDE_DIR}")
link_directories("${BOOST_LIB_DIR}")

set(Boost_LIBRARIES "${BOOST_LIB_DIR}/libboost_thread.so" "${BOOST_LIB_DIR}/libboost_system.so" "${BOOST_LIB_DIR}/libboost_chrono.so" "${BOOST_LIB_DIR}/libboost_unit_test_framework.so")

# Boost Release (for performance tests)
set(BOOST_RELEASE_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/boostrelease-prefix/src/boostrelease/")
set(BOOST_RELEASE_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/boostreleaseinstall")
set(BOOST_RELEASE_INCLUDE_DIR "${BOOST_RELEASE_INSTALL_DIR}/include")
set(BOOST_RELEASE_LIB_DIR "${BOOST_RELEASE_INSTALL_DIR}/lib")

ExternalProject_Add(
	boostrelease
	URL "https://sourceforge.net/projects/boost/files/boost/${boost_VERSION}/boost_${boost_VERSION_UNDERSCORE}.tar.bz2"
	INSTALL_COMMAND "" # Disable install step
	UPDATE_COMMAND "" # Doesn't change often
	CONFIGURE_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/configureboost.sh" "${BOOST_RELEASE_DOWNLOAD_DIR}" "${BOOST_RELEASE_INSTALL_DIR}"
	BUILD_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/buildboost_release.sh" "${BOOST_RELEASE_DOWNLOAD_DIR}" "${BOOST_RELEASE_LIB_DIR}"
	LOG_DOWNLOAD ON
	LOG_CONFIGURE ON
	LOG_BUILD ON
)

ExternalProject_Add_StepDependencies(boostrelease configure "${CMAKE_CURRENT_SOURCE_DIR}/configureboost.sh")
ExternalProject_Add_StepDependencies(boostrelease build "${CMAKE_CURRENT_SOURCE_DIR}/buildboost_release.sh")

set(Boost_RELEASE_LIBRARIES "${BOOST_RELEASE_LIB_DIR}/libboost_thread.so" "${BOOST_RELEASE_LIB_DIR}/libboost_system.so" "${BOOST_RELEASE_LIB_DIR}/libboost_chrono.so")

# Folly

# See https://github.com/facebook/folly/blob/master/README.md for all dependencies which have to be installed on Fedora as well.
find_package(gflags REQUIRED)
find_library(GLOG_LIBRARY glog)

if (NOT GLOG_LIBRARY)
	message(FATAL_ERROR "Library glog not found.")
endif ()

set(folly_DIR "${CMAKE_CURRENT_BINARY_DIR}/folly-prefix/src/folly/folly")
set(folly_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installfolly")
set(folly_INCLUDE_DIR "${folly_INSTALL_DIR}/include")
set(folly_LIB_DIR "${folly_INSTALL_DIR}/lib")

set(folly_LIBRARIES
	"${folly_LIB_DIR}/libfolly.so"
	${GLOG_LIBRARY}
)

ExternalProject_Add(folly
	URL "https://github.com/facebook/folly/archive/${folly_VERSION}.tar.gz"
	INSTALL_COMMAND ""
	CONFIGURE_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/configurefolly.sh" "${folly_DIR}" "${folly_INSTALL_DIR}" "${folly_LIB_DIR}/libfolly.so" "${BOOST_INSTALL_DIR}"
	BUILD_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/buildfolly.sh" "${folly_DIR}" "${folly_INSTALL_DIR}" "${folly_LIB_DIR}/libfolly.so"
	DEPENDS boost
)

include_directories("${folly_INCLUDE_DIR}")
link_directories("${folly_LIB_DIR}")

# Folly Release
set(folly_RELEASE_DIR "${CMAKE_CURRENT_BINARY_DIR}/follyrelease-prefix/src/follyrelease/folly")
set(folly_RELEASE_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installfollyrelease")
set(folly_RELEASE_INCLUDE_DIR "${folly_RELEASE_INSTALL_DIR}/include")
set(folly_RELEASE_LIB_DIR "${folly_RELEASE_INSTALL_DIR}/lib")

set(folly_RELEASE_LIBRARIES
	"${folly_RELEASE_LIB_DIR}/libfolly.so"
	${GLOG_LIBRARY}
)

ExternalProject_Add(follyrelease
	URL "https://github.com/facebook/folly/archive/${folly_VERSION}.tar.gz"
	INSTALL_COMMAND ""
	CONFIGURE_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/configurefolly_release.sh" "${folly_RELEASE_DIR}" "${folly_RELEASE_INSTALL_DIR}" "${folly_RELEASE_LIB_DIR}/libfolly.so" "${BOOST_RELEASE_INSTALL_DIR}"
	BUILD_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/buildfolly.sh" "${folly_RELEASE_DIR}" "${folly_RELEASE_INSTALL_DIR}" "${folly_RELEASE_LIB_DIR}/libfolly.so"
	DEPENDS boostrelease
)

# Wangle is required for the Thread Pool Executors which work with Folly
set(wangle_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installwangle")
set(wangle_BUILD_DIR "${CMAKE_BINARY_DIR}/wangle-prefix/src/wangle-build")

ExternalProject_Add(wangle
	URL "https://github.com/facebook/wangle/archive/${wangle_VERSION}.tar.gz"
	INSTALL_COMMAND ""
	CONFIGURE_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/configurewangle.sh" "${folly_LIB_DIR}" "${folly_INCLUDE_DIR}" "${BOOST_INSTALL_DIR}" "${wangle_INSTALL_DIR}"
	BUILD_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/buildwangle.sh" "${wangle_BUILD_DIR}"
	DEPENDS folly boost
)

set(wangle_INCLUDE_DIR "${wangle_INSTALL_DIR}/include/")
set(wangle_LIB_DIR "${wangle_INSTALL_DIR}/lib/")
set(wangle_LIBRARIES "${wangle_LIB_DIR}/libwangle.so")

include_directories("${wangle_INCLUDE_DIR}")

# Wangle Release:
set(wangle_RELEASE_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/installwanglerelease")
set(wangle_RELEASE_BUILD_DIR "${CMAKE_BINARY_DIR}/wanglerelease-prefix/src/wangle-build")

ExternalProject_Add(wanglerelease
	URL "https://github.com/facebook/wangle/archive/${wangle_VERSION}.tar.gz"
	INSTALL_COMMAND ""
	CONFIGURE_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/configurewangle_release.sh" "${folly_RELEASE_LIB_DIR}" "${folly_RELEASE_INCLUDE_DIR}" "${BOOST_RELEASE_INSTALL_DIR}" "${wangle_RELEASE_INSTALL_DIR}"
	BUILD_COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/buildwangle.sh" "${wangle_RELEASE_BUILD_DIR}"
	DEPENDS follyrelease boostrelease
)

set(wangle_RELEASE_INCLUDE_DIR "${wangle_RELEASE_INSTALL_DIR}/include/")
set(wangle_RELEASE_LIB_DIR "${wangle_RELEASE_INSTALL_DIR}/lib/")
set(wangle_RELEASE_LIBRARIES "${wangle_RELEASE_LIB_DIR}/libwangle.so")

# The file compile_commands.json is require by clang-check and has to be exported.
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=1)

find_program(CLANG_CHECK_PATH clang-check)

if (NOT CLANG_CHECK_PATH)
	message(FATAL_ERROR "clang-check not found.")
endif ()

find_program(VALGRIND_PATH valgrind)

if (NOT VALGRIND_PATH)
	message(FATAL_ERROR "valgrind not found.")
endif ()

find_program(GPROF_PATH gprof)

if (NOT GPROF_PATH)
	message(FATAL_ERROR "gprof not found.")
endif ()


find_program(BASH_PATH bash)

if (NOT BASH_PATH)
	message(FATAL_ERROR "bash not found.")
endif ()

add_subdirectory(src)

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