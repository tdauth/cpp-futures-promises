#!/bin/bash
CC="/usr/bin/gcc"
CXX="/usr/bin/g++"

cmake ../wangle/wangle -DFOLLY_LIBRARYDIR="$1" -DFOLLY_INCLUDEDIR="$2" -DBUILD_SHARED_LIBS=ON -DBOOST_ROOT="$3" -DBOOST_INCLUDEDIR="$3/include" -DBOOST_LIBRARYDIR="$3/lib" -DBoost_NO_SYSTEM_PATHS="TRUE" -DBoost_NO_BOOST_CMAKE="TRUE" -DCMAKE_BUILD_TYPE="Debug" -DCMAKE_INSTALL_PREFIX="$4" -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CXXFLAGS" -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX"