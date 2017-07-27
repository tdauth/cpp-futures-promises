#!/bin/bash
# Exclude the build directory:
find . \( -path ./build -prune -o -name "*.cpp" -o -name "*.h" \) -print0 | xargs -0 clang-format