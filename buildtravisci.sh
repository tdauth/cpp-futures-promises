#!/bin/bash
echo "Installing dependencies on Fedora:"
bash ./install_fedora_dependencies.sh

echo "Building project in debug mode:"
bash ./build.sh

echo "Building project in coverage mode:"
bash ./buildcoverage.sh

echo "Building project in release mode:"
bash ./buildrelease.sh

echo "Cppcheck analysis:"
bash ./cppcheck.sh