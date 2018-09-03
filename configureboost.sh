#!/bin/bash

if [ -z "$1" ] ; then
	echo "Specify the directory where the bootstrap.sh script is located."
	exit 1
fi

if [ -z "$2" ] ; then
	echo "Specify the prefix directory where the Boost C++ Libraries should be build."
	exit 1
fi

cd "$1"
# thread is required for the futures and promises component
# chrono is required for time measurements
# context, regex, filesystem and program_options are required by Folly
# test is required for all unit tests
./bootstrap.sh --with-toolset=gcc --with-libraries=thread,chrono,context,program_options,regex,filesystem,test,date_time --prefix="\"$2\""