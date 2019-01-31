#!/bin/bash
cd "$1"
./b2 dll-path="\"$2\"" cxxflags="$3 -stdlib=libc++" linkflags="-stdlib=libc++" -j4 toolset=clang variant="$4" install
# TODO apparently bjam ignores the rpath on installing and Folly fails at the linking step with the Boost C++ Libraries since to rpaths are set.
echo "Changing rpaths of Boost libraries:"
cd "$2"
echo "Listing all files:"
ls -lha