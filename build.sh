#!/bin/bash

if [ ! -d "./build" ] ; then
	mkdir ./build
fi

CC="/usr/bin/gcc"
CXX="/usr/bin/g++"

echo "CC is set to $CC"
echo "CXX is set to $CXX"

# Configure and build everything:
cd ./build
# Create a "compile_commands.json" file for analysis: http://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools
cmake ../ -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX"
make -j1 # Use one job to improve error detection and exit early.

# Show some build logs for TravisCI:
if [ "$?" -ne 0 ] ; then
	exit 1
fi

# Delete all unit test log files and run all unit tests:
rm -r "./Testing" || true
# Pass longer timeouts since the shared test may take longer especially with a memcheck:
ctest -T test --timeout 5000
ctest -T memcheck --timeout 5000
ctest -T coverage --timeout 5000

# Create a package:
cpack -G "RPM" .