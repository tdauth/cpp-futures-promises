#!/bin/bash
cd "$1"
./b2 dll-path="$2" cxxflags="-std=c++1z -O3" -j4 toolset=gcc variant=release install
# TODO apparently bjam ignores the rpath on installing and Folly fails at the linking step with the Boost C++ Libraries since to rpaths are set.
echo "Changing rpaths of Boost libraries:"
cd "$2"
echo "Listing all files:"
ls -lha
for filename in "$2"/*.so; do
	echo "Current rpath of Boost Library $filename:"
	chrpath -l "$filename"
	echo "Changing rpath of Boost Library $filename to \"$2\"."
	patchelf --set-rpath "$2" "$filename"
done