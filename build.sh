#!/bin/bash

if [ ! -d "./build" ] ; then
	mkdir ./build
fi

# Use the local CMake modules for TravisCI:
CMAKE_MODULE_PATH="$(pwd)/cmake"
echo "Using prefix path: $CMAKE_MODULE_PATH"
echo "It contains the following files:"
ls -lha "$CMAKE_MODULE_PATH"

# Configure and build everything:
cd ./build
# Create a "compile_commands.json" file for analysis: http://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools
cmake ../ -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_CXX_COMPILER="/usr/bin/g++" -DCMAKE_C_COMPILER="/usr/bin/gcc"
make -j4

# Show some build logs for TravisCI:
if [ "$?" -ne 0 ] ; then
	cat ./build/boost-prefix/src/boost-stamp/boost-build-*.log
	cat ./build/boostrelease-prefix/src/boostrelease-stamp/boostrelease-build-*.log
	exit 1
fi

# Delete all unit test log files and run all unit tests:
rm -r "./Testing" || true
ctest -T test

# Create a package:
cpack -G "RPM" .