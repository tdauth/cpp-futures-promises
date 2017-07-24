#!/bin/bash

# Skip configuring if the library does still exist. This improves the performance on every compilation. Just delete the build directory to rebuild it.
if [ -e "$3" ] ; then
	echo "File $3 does still exist. Don't reconfigure to improve the build performance. Clean the whole build dir to build it new."
	exit 0
fi

if [ ! -d "$4" ]; then
	echo "Specify the Boost root dir of the current build."
	exit 1
fi

echo "Boost root dir $4:"
ls -lha "$4"

cd "$1"
export LDFLAGS="-L\"$4/lib/\" -Wl,-rpath=\"$4/lib/\" -Wl,-rpath-link=\"$4/lib/\""
echo "LDFLAGS: $LDFLAGS"
autoreconf -ivf
CFLAGS="$CFLAGS -O3" CXXFLAGS="$CXXFLAGS -O3" ./configure --prefix="$2" --with-boost="$4" --with-boost-libdir="$4/lib/"