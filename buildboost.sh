#!/bin/bash
cd "$1"
./b2 dll-path="\"$2\"" cxxflags=-std=c++1z -j4 toolset=clang variant=debug install
# TODO apparently bjam ignores the rpath on installing and Folly fails at the linking step with the Boost C++ Libraries since to rpaths are set.
echo "Changing rpaths of Boost libraries:"
cd "$2"
echo "Listing all files:"
ls -lha