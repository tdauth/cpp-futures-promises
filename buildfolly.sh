#!/bin/bash

# Skip building if the library does still exist. This improves the performance on every compilation. Just delete the build directory to rebuild it.
if [ -e "$3" ] ; then
	echo "File $3 does still exist. Don't build to improve the build performance."
	exit 0
fi

cd "$1"
make -j4 prefix="$2" install