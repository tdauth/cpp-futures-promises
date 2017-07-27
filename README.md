[![Build Status](https://travis-ci.org/tdauth/cpp-futures-promises.svg?branch=master)](https://travis-ci.org/tdauth/cpp-futures-promises)

# cpp-futures-promises
Examples and extensions of C++ futures and promises based on C++17, Boost.Thread and Folly.
The extensions are mainly based on the Scala library for futures and promises: http://docs.scala-lang.org/overviews/core/futures.html

## Compile
To compile the project run the following script on Linux:
`bash ./build.sh`
It will compile the project, run all unit tests and create an RPM package.
Note that all targets are added as CTest unit tests which makes their execution easier.

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
* orElse for Folly and Boost.Thread
* first
* firstOnlySucc
* firstSucc
* firstSucc2
* collectNWithoutException for Folly.
* whenN for Boost.Thread
* whenAny for Boost Thread which returns a pair of the completed future and its index.

### Promise extensions
Besides, extensions for promises are provided which do exist in Scala:
* tryComplete
* tryCompleteWith
* tryCompleteSuccess
* tryCompleteSuccessWith
* tryCompleteFailure
* tryCompleteFailureWith
* completeWith
* fromTry
* failed
* successful

### Unit Tests
To test the functionality, several unit tests are provided for the extensions.
Bear in mind that concurrent programs can behavior differently every time due to scheduling.
Therefore, simple unit tests won't proof that there are no bugs.

## Performance Tests
The project provides several performance tests using the benchmark suite from Folly:
* Shared vs unique future and promise creation
* Recursive non-blocking combinator calls

## Use Cases
The project provides several use cases of futures and promises.

### Currency Exchange
For demonstrating the libraries, a simple use case of exchanging currencies is demonstrated for all three libraries.
It is extended with guards and finally a choice between two exchanges, but only for Folly.
This use case shows the limitations of the libraries.

### Santa Claus
Santa Claus sleeps and is woken up either by a group of reindeer or a group of elves.
In our variant he prefers to wait for the reindeer first and only if they could not make it he waits for the elves.
This is realiezd with the usage of orElse.
Depending on which group wakes up Santa, he either delivers toys or constructs them.

[Original Paper](http://dl.acm.org/citation.cfm?id=187391)