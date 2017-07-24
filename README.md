[![Build Status](https://travis-ci.org/tdauth/cpp-futures-promises.svg?branch=master)](https://travis-ci.org/tdauth/cpp-futures-promises)

# cpp-futures-promises
Examples and extensions of C++ futures and promises based on C++17, Boost.Thread and Folly.
The extensions are mainly based on the Scala library for futures and promises: http://docs.scala-lang.org/overviews/core/futures.html

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

## Use Case: Currency Exchange
For demonstrating the libraries, a simple use case of exchanging currencies is demonstrated for all three libraries.
It is extended with guards and finally a choice between two exchanges, but only for Folly.
This use case shows the limitations of the libraries.

## Extensions
The project provides a number of non-blocking combinators which are missing from Folly (for two futures only):
* firstSuccess()
* firstSuccessRandom()
* ...
* orElse()
* collectNWithoutException() for Folly.
* when_n() for Boost.Thread

## Performance Tests
THe project provides several performance tests using the benchmark suite from Folly:
* Shared vs unique future and promise creation
* Recursive non-blocking combinator calls