#!/bin/bash

BUILD_DIR="./build_release"

if [ ! -d "$BUILD_DIR" ] ; then
	mkdir "$BUILD_DIR"
fi

CC="/usr/bin/gcc"
CXX="/usr/bin/g++"

# Configure and build everything:
cd "$BUILD_DIR"
cmake ../ -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_BUILD_TYPE="Release" -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX"
make -j1 # Use one job to improve error detection and exit early.

# Show some build logs for TravisCI:
if [ "$?" -ne 0 ] ; then
	exit 1
fi

# Create a package:
cpack -G "RPM" .