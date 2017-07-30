#!/bin/bash

if [ ! -d "./build" ] ; then
	mkdir ./build
fi

echo "CC is set to $CC"
echo "CXX is set to $CXX"
echo "clang is at $(which clang)"
echo "clang++ is at $(which clang++)"

# Use the local CMake modules for TravisCI:
CMAKE_MODULE_PATH="$(pwd)/cmake"
echo "Using prefix path: $CMAKE_MODULE_PATH"
echo "It contains the following files:"
ls -lha "$CMAKE_MODULE_PATH"

# Configure and build everything:
cd ./build
# Create a "compile_commands.json" file for analysis: http://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools
cmake ../ -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_CXX_COMPILER="/usr/bin/clang++" -DCMAKE_C_COMPILER="/usr/bin/clang"
make -j1 # Use one job to improve error detection and exit early.

# Show some build logs for TravisCI:
if [ "$?" -ne 0 ] ; then
	ls -lha ./boost-prefix/src/boost-stamp/
	cat ./boost-prefix/src/boost-stamp/boost-build-*.log
	ls -lha ./boostrelease-prefix/src/boostrelease-stamp/
	cat ./boostrelease-prefix/src/boostrelease-stamp/boostrelease-build-*.log

	cat ./folly-prefix/src/folly/folly/config.log

	exit 1
fi

# Delete all unit test log files and run all unit tests:
rm -r "./Testing" || true
# Pass longer timeouts since the shared test may take longer especially with a memcheck:
ctest -T test --timeout 5000
ctest -T memcheck --timeout 5000

# Create a package:
cpack -G "RPM" .