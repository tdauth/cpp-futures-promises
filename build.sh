#!/bin/bash

if [ ! -d "./build" ] ; then
	mkdir ./build
fi

CMAKE_MODULE_PATH="$(pwd)/cmake"
echo "Using prefix path: $CMAKE_MODULE_PATH"
echo "It contains the following files:"
ls -lha "$CMAKE_MODULE_PATH"
cd ./build
# Clang cannot be used due to errors with Folly: https://github.com/facebook/folly/issues/555
# Create a "compile_commands.json" file for analysis: http://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools
cmake ../ -DCMAKE_MODULE_PATH="$CMAKE_MODULE_PATH" -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_CXX_COMPILER="/usr/bin/g++" -DCMAKE_C_COMPILER="/usr/bin/gcc"
make -j4
rm -r "$WORKSPACE/cpp-futures-promises/workspace/build/Testing" || true
ctest -T test
cpack -G "RPM" .