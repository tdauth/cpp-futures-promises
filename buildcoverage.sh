#!/bin/bash

BUILD_DIR="./build_coverage"

if [ ! -d "$BUILD_DIR" ] ; then
	mkdir "$BUILD_DIR"
fi

CC="/usr/bin/gcc"
CXX="/usr/bin/g++"
GCC_COVERAGE_COMPILE_FLAGS="-g -O0 -coverage -fprofile-arcs -ftest-coverage"
GCC_COVERAGE_LINK_FLAGS="-coverage -lgcov"

# Configure and build everything:
cd "$BUILD_DIR"
# Create a "compile_commands.json" file for analysis: http://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools
cmake ../ -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" -DCMAKE_CXX_FLAGS="$GCC_COVERAGE_COMPILE_FLAGS" -DCMAKE_EXE_LINKER_FLAGS="$GCC_COVERAGE_LINK_FLAGS"
make -j1 # Use one job to improve error detection and exit early.

# Show some build logs for TravisCI:
if [ "$?" -ne 0 ] ; then
	exit 1
fi

# Delete all unit test log files and run all unit tests:
rm -r "./Testing" || true

# Empty coverage data:
lcov --zerocounters  --directory .

# Pass longer timeouts since the shared test may take longer:
ctest -T test --timeout 5000

# Collect coverage data:
lcov --directory . --capture --output-file my_prog.info
# Remove coverage data from test code and external libraries:
lcov --remove my_prog.info '/usr/include/*' '/usr/lib/*' '*/build_coverage/folly_install/*' \
'*/build_coverage/boost_install/*' '*/src/extensions.cpp' '*/src/folly_fixture.h' '*/src/performance/*' '*/src/advanced/boost/test/*' '*/src/advanced/folly/test/*' \
-o my_prog_filtered.info
# Generate coverage HTML output:
genhtml --output-directory coverage \
  --demangle-cpp --num-spaces 2 --sort \
  --title "Advanced Futures and Promises Test Coverage" \
  --function-coverage --branch-coverage --legend \
  my_prog_filtered.info