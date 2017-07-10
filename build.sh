#!/bin/bash

if [ ! -d "./build" ] ; then
	mkdir ./build
fi

cd ./build
# Clang cannot be used due to errors with Folly: https://github.com/facebook/folly/issues/555
# Create a "compile_commands.json" file for analysis: http://eli.thegreenplace.net/2014/05/21/compilation-databases-for-clang-based-tools
cmake ../ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_CXX_COMPILER="/usr/bin/clang++" -DCMAKE_C_COMPILER="/usr/bin/clang" -DCMAKE_CXX_COMPILER="/usr/bin/clang++" -DCMAKE_C_COMPILER="/usr/bin/clang"
make -j4
ctest
cpack -G "RPM" .