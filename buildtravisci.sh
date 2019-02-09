#!/bin/bash

echo "Building project in coverage mode:"
bash ./buildcoverage.sh

echo "Cppcheck analysis:"
bash ./cppcheck.sh