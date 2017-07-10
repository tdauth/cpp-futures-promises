#!/bin/bash
# CC="/usr/bin/clang" CXX="/usr/bin/clang++"
# GCC cannot compile folly (syntax errors):
# CC=/usr/bin/gcc CXX=/usr/bin/g++
# CC="/usr/bin/clang" CXX="/usr/bin/clang++"
# NOTE Apparently the compiler from CMake is used here.

# Skip configuring if the library does still exist. This improves the performance on every compilation. Just delete the build directory to rebuild it.
if [ -e "$3" ] ; then
	echo "File $3 does still exist. Don't reconfigure to improve the build performance. Clean the whole build dir to build it new."
	exit 0
fi

if [ ! -d "$4" ]; then
	echo "Specify the Boost root dir of the current build."
	exit 1
fi

cd "$1"
export LDFLAGS="-L\"$4/lib/\" -Wl,-rpath=\"$4/lib/\" -Wl,-rpath-link=\"$4/lib/\""
echo "LDFLAGS: $LDFLAGS"
autoreconf -ivf
CFLAGS='-std=c11' CXXFLAGS='-std=c++14' ./configure --prefix="$2" --with-boost="$4" --with-boost-libdir="$4/lib/"
# FIXME Clang build crashes
#  CC=clang CXX=clang++ CFLAGS=-std=c11 CXXFLAGS='-std=c++14 -msse4.2'
