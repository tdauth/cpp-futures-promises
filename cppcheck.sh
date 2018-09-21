#!/bin/bash
cppcheck --force --xml --enable=all --inconclusive --language=c++ --std=c++14 --platform=unix64 -I ./build/boost_install/include/ -I ./build/folly_install/include/ ./src  2> result.xml &&\
cppcheck-htmlreport --source-encoding="utf8" --title="Advanced Futures and Promises" --source-dir=. --report-dir=cppcheck_report --file=result.xml && rm result.xml