#!/bin/bash
echo "Installing dependencies on Fedora:"
bash ./install_fedora_dependencies.sh

echo "Building project in debug mode:"
bash ./build.sh

echo "Building project in coverage mode:"
bash ./build_coverage.sh

echo "Building project in release mode:"
bash ./build_release.sh

echo "Cppcheck analysis:"
bash ./cppcheck.sh

echo "Update online coverage reports:"
bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports""