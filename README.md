# cpp-futures-promises
Examples  and extensions of C++ futures and promises based on C++17, Boost.Thread and Folly.

## Compile
To compile the project run the following script on Linux:
`bash ./build.sh`
It will compile the project, run all unit tests and create an RPM package.

## Dependencies
The project requires Clang with C++17 support and CMake to be built.

The project requires the following libraries:
* Boost
* Folly
* Wangle

It will download and compile these libraries automatically when being compiled.
The versions of the libraries are specified in the CMakeLists.txt file in the top level directory of the project.

## Extensions
The project provides a number of non-blocking combinators which are missing from Folly (for two futures only):
* firstSuccess()
* firstSuccessRandom()
* ...
* orElse()